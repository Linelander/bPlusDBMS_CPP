// HEADER:
//   1    +   1    +      4       +     4
// isLeaf, rootBool, numSignposts, numChildren, signposts (T), children (size_t)

#include <cstddef>
#include <cstdio>
#include<iostream>
#include <stdexcept>
#include <unistd.h>
#include "BPNode.h"
#include "Bufferpool.h"
#include <memory>
#include "Utils.h"


class ItemInterface;

template<typename T> class BPlusTreeBase;

template<typename T, int way> class BPLeaf;

using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

static const size_t INVALID_PAGE_ID = -1;

template<typename T, int way>
class BPInternalNode : public BPNode<T, way> {
    // Header: sizeof(isLeaf) + sizeof(itemKeyIndex)
    private:
        bool isLeaf = false;
        int itemKeyIndex; // "index" of column we're creating an index on
        std::shared_ptr<BPlusTreeBase<int>> clusteredIndex;
        int columnCount;
        bool rootBool{false};
        int pageSize = 4096;
        int signCapacity{};
        int numSignposts{};
        int numChildren{};
        
        // Disk
        size_t pageOffset;
        Bufferpool<T, way>* bufferpool;
        size_t page;
        
        bool isOverFull() {return numSignposts > signCapacity;}
        
        std::array<size_t, way+1> children;

        std::array<T, way> signposts;
        // NOTE: using 1 dummy slot at the end of each array for splitting logic.




    public:
        vector<uint8_t> getBytes() {
            std::vector<uint8_t> bytes;

            Utils::appendBytes(bytes, isLeaf);        // 1 byte
            Utils::appendBytes(bytes, rootBool);      // 1 byte
           
            Utils::appendBytes(bytes, numSignposts);  // 4 bytes
            Utils::appendBytes(bytes, numChildren);   // 4 bytes

            for(int i = 0; i < numSignposts; i++) {
                Utils::appendBytes(bytes, signposts[i]);
            }
            

            for(int i = 0; i < numChildren; i++) {
                Utils::appendBytes(bytes, children[i]);  // size_t bytes * numChildren
            }
            
            return bytes;
        }




        size_t getPageOffset() {
            return page;
        }




        // ~BPInternalNode() {
        //     for (int i = 0; i < numChildren; i++) {
        //         bufferpool->freePage(children[i]); // TODO: uhh....
        //     }
        // }




        void dehydrate() {
            int fd = bufferpool->getFileDescriptor();
            size_t offset = getPageOffset();
            vector<uint8_t> bytes = getBytes();

            lseek(fd, offset, SEEK_SET);

            checkRW(write(fd, bytes.data(), bytes.size()), fd);

            // Bufferpool calls delete. should delete do anything extra?
        }

        


        // DISK
        size_t getPage(){return page;}

        void giveOffset(size_t offset) {
            this->page = offset;
        }
    
    


        // CONSTRUCTORS / DEST.
        BPInternalNode(const int keyIndex, const int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool) {
            itemKeyIndex = keyIndex;
            pageSize = sysconf(_SC_PAGESIZE);
            this->signCapacity = way-1;
            bufferpool = bPool;
            children.fill(INVALID_PAGE_ID);
            columnCount = colCount;
            clusteredIndex = std::move(mainTree);
            page = bufferpool->allocate(this);
        }




        BPInternalNode(const int keyIndex, const int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, size_t nonstandardSize) {
            itemKeyIndex = keyIndex;
            pageSize = nonstandardSize;
            this->signCapacity = way-1;
            bufferpool = bPool;
            children.fill(INVALID_PAGE_ID);
            columnCount = colCount;
            clusteredIndex = std::move(mainTree);
            page = bufferpool->allocate(this);
        }




        BPInternalNode(const int keyIndex, const int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, vector<T> sPosts, vector<size_t> chldrn) {
            itemKeyIndex = keyIndex;
            pageSize = sysconf(_SC_PAGESIZE);
            this->signCapacity = way-1;
            bufferpool = bPool;
            children.fill(INVALID_PAGE_ID);
            columnCount = colCount;
            clusteredIndex = std::move(mainTree);
            page = bufferpool->allocate(this);

            for (int i = 0; i < sPosts.size(); i++) {
                signposts[i] = sPosts[i];
            }

            for (int i = 0; i < sPosts.size(); i++) {
                children[i] = chldrn[i];
            }
        }




        BPInternalNode(const int keyIndex, const int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, vector<T> sPosts, vector<size_t> chldrn, size_t nonstandardSize) {
            itemKeyIndex = keyIndex;
            pageSize = nonstandardSize;
            this->signCapacity = way-1;
            bufferpool = bPool;
            children.fill(INVALID_PAGE_ID);
            columnCount = colCount;
            clusteredIndex = std::move(mainTree);
            page = bufferpool->allocate(this);

            for (int i = 0; i < sPosts.size(); i++) {
                signposts[i] = sPosts[i];
            }

            for (int i = 0; i < sPosts.size(); i++) {
                children[i] = chldrn[i];
            }
        }


        

        // METHODS
        bool isRoot() {return rootBool;}
        void makeRoot() {rootBool = true;}
        void notRoot() {rootBool = false;}
        bool isLeafFn() {return false;}

        void printKey(int key) {
            cout << key;
        }




        void printKey(const AttributeType& attr) {
            cout << attr.data();
        }




        int getNumChildren() {
            return numChildren;
        }




        void setPageOffset(size_t offset) {
            pageOffset = offset;
        }



        void receiveItem(ItemInterface* newItem) {
            throw std::runtime_error("Internal nodes can't receive items");
        }




        ItemInterface* giveUpFirstItem() {
            throw std::runtime_error("Internal nodes can't give items");
        }



        ItemInterface* giveUpLastItem() {
            throw std::runtime_error("Internal nodes can't give items");
        }




        // Stacks a child and sign on the front of the children and sign array
        void receiveChild(size_t givenChild, T givenPost) {
            insertChild(givenChild, 0);
            insertSignpost(givenPost, 0);
        }




        void giveChild(BPInternalNode* receiver) {
            if (numChildren == 0) {
                throw std::underflow_error("No children to give");
            }
            if (numSignposts == 0) {
                throw std::underflow_error("Attempted to give child with no signposts");
            }
            if (receiver == nullptr) {
                throw std::invalid_argument("Attempted to give child to null receiver");
            }
            
            size_t popChild = children[numChildren - 1];
            children[numChildren - 1] = INVALID_PAGE_ID;
            numChildren--;
            
            T popPost = signposts[numSignposts - 1];
            signposts[numSignposts - 1] = T{};
            numSignposts--;
            
            receiver->receiveChild(popChild, popPost);
        }




        /*
        This method will split our node, creating a new sibling
        The new sibling will have an extra signpost. 
        It is the responsibility of other methods to steal this signpost for the parent (see promote)
        */
        size_t split() {
            // redistribute children to a new node
            // const int keyIndex, const int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, size_t nonstandardSize
            BPInternalNode* siblingNode = new BPInternalNode(itemKeyIndex, columnCount, clusteredIndex, bufferpool, pageSize);
            size_t sibling = siblingNode->getPageOffset();
            while (siblingNode->getNumChildren() != this->numChildren+1 && siblingNode->getNumChildren() != this->numChildren) {
                giveChild(siblingNode);
            }

            return sibling;
        }




        void insertSignpost(const T& key, int pos) {
            if (pos >= signposts.size())
            {
                throw std::out_of_range("Signpost pos out of range");
            }
            for (int i = numSignposts; i > pos; i--) {
                signposts[i] = signposts[i-1];
            }
            signposts[pos] = key;
            numSignposts++;
        }




        void insertChild(size_t child, int pos) {
            if (pos >= children.size())
            {
                throw std::out_of_range("Child pos out of range");
            }
            for (int i = numChildren; i > pos; i--) {
                children[i] = children[i-1];
            }
            children[pos] = child;
            numChildren++;
        }




        void removeChildAt(int pos) {
            if (pos < 0 || pos >= numChildren) {
                throw std::out_of_range("Child pos out of range");
            }
            
            for (int i = pos; i < numChildren - 1; i++) {
                children[i] = children[i + 1];
            }
            
            children[numChildren - 1] = INVALID_PAGE_ID;
            numChildren--;
        }
    



        // Remove signpost at specific position
        void removeSignpostAt(int pos) {
            if (pos < 0 || pos >= numSignposts) {
                throw std::out_of_range("Signpost pos out of range");
            }
            
            for (int i = pos; i < numSignposts - 1; i++) {
                signposts[i] = signposts[i + 1];
            }
            
            signposts[numSignposts - 1] = T{};
            numSignposts--;
        }




        T viewSign1() {
            return signposts[0];
        }




        T getSign1() {
            T result = signposts[0];
            removeSignpostAt(0);
            return result;
        }




        /* Handles adding new children created by splits to the children list.
            This is also where keys are stolen during splits.

            Uses the signpost insertion position to place the child without additional
            disk accesses: signposts[i] = min key of children[i+1], so a new child
            whose min key lands at signpost position p goes into children[p+1].
        */
        void sortedInsert(size_t newChild) {
            BPNode<T, way>* newChildNode = bufferpool->getNode(newChild);
            T newSign{};
            if (newChildNode->isLeafFn()) {
                newSign = newChildNode->viewSign1(); // leaf split: copy key, don't steal
            } else {
                newSign = newChildNode->getSign1();  // internal split: steal first key
            }
            bufferpool->freePage(newChild);

            // Find insertion position in signposts array (in-memory, no disk I/O)
            int signPos = numSignposts;
            for (int i = 0; i < numSignposts; i++) {
                if (signposts[i] > newSign) {
                    signPos = i;
                    break;
                }
            }
            insertSignpost(newSign, signPos);
            // signposts[p] == min key of children[p+1], so child goes at signPos+1
            insertChild(newChild, signPos + 1);
        }

        void becomeInternalRoot(std::array<size_t, 2> newChildren) {
            insertChild(newChildren[0], 0); // FLAG
            sortedInsert(newChildren[1]);
            makeRoot();
        }




        /* The method parents use to steal/copy keys from newborn children
            Return value of null: no split occured
        */
        size_t promote(size_t rep) {
            sortedInsert(rep);

            size_t splitResult = INVALID_PAGE_ID;
            if (isOverFull())
            {
                splitResult = split();

                if (isRoot()) {
                    BPInternalNode* newRoot = new BPInternalNode<T, way>(itemKeyIndex, columnCount, clusteredIndex, bufferpool, pageSize);
                    std::array<size_t, 2> rootChildren = {page, splitResult};
                    // first keys are stolen by a call to sorted insert inside this method
                    newRoot->becomeInternalRoot(rootChildren);
                    this->notRoot();
                    splitResult = newRoot->getPageOffset();
                    return splitResult;
                }

                return splitResult;
            }

            // Case 3: We do not split. Null represents no split.
            return INVALID_PAGE_ID;
        }




        void becomeFirstInternalRoot(std::array<size_t, 2> newChildren) {
            insertChild(newChildren[0], numChildren);
            insertChild(newChildren[1], numChildren);
            insertSignpost(bufferpool->getNode(newChildren[1])->viewSign1(), numSignposts);
        }
        


        
        int getChildIndexByKey(T key) {
            int left = 0;
            int right = numSignposts;
            while (left < right) {
                int mid = left + (right-left) / 2;
                if (signposts[mid] <= key)
                {
                    left = mid + 1;
                }
                else right = mid;
            }
            return left;
        }
        
        


        /* 
            When inserting on internal nodes that are children, add the result of insertion to the children list IF its pointer is different from the one you inserted on.
            After a recursive call resulting in a split, promote handles the copying/stealing of the new child's key (whichever is needed)            
        */ 
        size_t insert(ItemInterface* newItem) {
            T newItemkey = any_cast<T>(newItem->dynamicGetKeyByIndex(itemKeyIndex));
            size_t childPageOffset = children[getChildIndexByKey(newItemkey)];
            
            size_t result = bufferpool->getNode(childPageOffset)->insert(newItem);

            bufferpool->freePage(childPageOffset);
            
            if (result == INVALID_PAGE_ID) {  // no split
                bufferpool->freePage(childPageOffset);  // simple insertion
                return INVALID_PAGE_ID;
            }
            
            // split - promote the new sibling
            size_t promoteResult = promote(result);
            bufferpool->freePage(childPageOffset);
            return promoteResult;
        }
        



        // TOD maybe this should check signposts instead. thinking about 2-3+ trees.
        bool checkUnderfull() {
            return (numChildren < children.size() / 2);
            // return (numSignposts < children.size() / 2);
        }



        
        bool isWealthy() {
            return (numChildren == (children.size() / 2) + 1);
        }
        



        size_t backSteal() {
            size_t result = children[numChildren-1];
            removeChildAt(numChildren-1);
            if (numSignposts > 0) {
                removeSignpostAt(numSignposts-1);
            }
            return result;
        }




        size_t frontSteal() {
            size_t result = children[0];
            removeChildAt(0);
            if (numSignposts > 0) {
                removeSignpostAt(0);
            }
            return result;
        }




        // Get the leftmost value from this subtree
        T getHardLeft() {
            size_t childPageId = children[0];
            T result = bufferpool->getNode(childPageId)->getHardLeft();
            bufferpool->freePage(childPageId);  // Simple query, free immediately
            return result;
        }




        // Rebuild all signposts from the hard-left of each child (used after merges).
        void generateSignposts() {
            while (numSignposts > 0) {
                removeSignpostAt(0);
            }
            for (int i = 1; i < numChildren; i++) {
                BPNode<T, way>* child = bufferpool->getNode(children[i]);
                T left = child->getHardLeft();
                bufferpool->freePage(children[i]);
                insertSignpost(left, i - 1);
            }
        }

        // Update just one signpost: signposts[pos] = hard-left of children[pos+1].
        void updateSignpost(int pos) {
            if (pos < 0 || pos >= numSignposts) return;
            BPNode<T, way>* child = bufferpool->getNode(children[pos + 1]);
            signposts[pos] = child->getHardLeft();
            bufferpool->freePage(children[pos + 1]);
        }




        void mergeLeftHere(BPNode<T, way>* dyingNode) {
            // Absorb dying node's children onto the right end.
            // Each stolen child needs a separator signpost = its own hard-left.
            while (dyingNode->getNumChildren() > 0) {
                size_t stolen = dyingNode->frontSteal();
                BPNode<T, way>* stolenNode = bufferpool->getNode(stolen);
                T stolenLeft = stolenNode->getHardLeft();
                bufferpool->freePage(stolen);
                insertSignpost(stolenLeft, numSignposts);
                insertChild(stolen, numChildren);
            }
        }




        void mergeRightHere(BPNode<T, way>* dyingNode) {
            // Absorb dying node's children onto our left end.
            // Build the full merged children list then regenerate all signposts,
            // since we're prepending and every signpost index shifts.
            while (dyingNode->getNumChildren() > 0) {
                insertChild(dyingNode->backSteal(), 0);
            }
            generateSignposts();
        }
        



        RemovalResult<T> handleUnderfull(RemovalResult<T> modifyResult, BPNode<T, way>* leftSiblingHere, BPNode<T, way>* rightSiblingHere) {
            bool structureChanged = false;

            // STEAL FROM LEFT
            if (leftSiblingHere != nullptr && leftSiblingHere->isWealthy()) {
                size_t stolen = leftSiblingHere->backSteal();
                insertChild(stolen, 0);
                // New children[1] is the old first child — add separator at front.
                BPNode<T, way>* newSecond = bufferpool->getNode(children[1]);
                T newSecondLeft = newSecond->getHardLeft();
                bufferpool->freePage(children[1]);
                insertSignpost(newSecondLeft, 0);

                modifyResult.action = RemovalAction::STOLE_FROM_LEFT;
            }

            // STEAL FROM RIGHT
            else if (rightSiblingHere != nullptr && rightSiblingHere->isWealthy()) {
                size_t stolen = rightSiblingHere->frontSteal();
                insertChild(stolen, numChildren);
                // Stolen child is the new last child — add separator at back.
                BPNode<T, way>* stolenNode = bufferpool->getNode(stolen);
                T stolenLeft = stolenNode->getHardLeft();
                bufferpool->freePage(stolen);
                insertSignpost(stolenLeft, numSignposts);

                modifyResult.action = RemovalAction::STOLE_FROM_RIGHT;
            }

            // MERGE WITH LEFT
            else if (leftSiblingHere != nullptr) {
                leftSiblingHere->mergeLeftHere(this);
                modifyResult.action = RemovalAction::MERGED_INTO_LEFT;
                bufferpool->deallocate(pageOffset);
            }

            // MERGE WITH RIGHT
            else if (rightSiblingHere != nullptr) {
                rightSiblingHere->mergeRightHere(this);
                modifyResult.action = RemovalAction::MERGED_INTO_RIGHT;
                bufferpool->deallocate(pageOffset);
            }
            
            modifyResult.lastLocation = LastLocation::INTERNAL;
            return modifyResult;
        }




        RemovalResult<T> remove(T deleteIt, BPNode<T, way>* leftSiblingHere, BPNode<T, way>* rightSiblingHere) {

            int childInd = getChildIndexByKey(deleteIt);
            int leftChildInd = childInd - 1;
            int rightChildInd = childInd + 1;

            // Save offsets before any structural changes so we can free correctly later.
            size_t childPage      = children[childInd];
            size_t leftChildPage  = (leftChildInd >= 0)          ? children[leftChildInd]  : (size_t)-1;
            size_t rightChildPage = (rightChildInd < numChildren) ? children[rightChildInd] : (size_t)-1;

            // Fetch siblings the child may need to steal from / merge with.
            BPNode<T, way>* leftSiblingDown  = nullptr;
            BPNode<T, way>* rightSiblingDown = nullptr;
            if (leftChildInd >= 0)          leftSiblingDown  = bufferpool->getNode(children[leftChildInd]);
            if (rightChildInd < numChildren) rightSiblingDown = bufferpool->getNode(children[rightChildInd]);

            // actual removal call
            RemovalResult<T> result = bufferpool->getNode(children[childInd])->remove(deleteIt, leftSiblingDown, rightSiblingDown);
            RemovalAction action = result.action;
            
            /*
                SWITCH BASED ON WHAT HAPPENED AT THE CHILD
            */
            switch (action) {
                case RemovalAction::DEFAULT:
                    throw std::runtime_error("Removal action should not be default at parent-child relationship management switch statement.");

                case RemovalAction::SIMPLE_REMOVAL:
                    // The child's leftmost key may have changed — update only the one signpost
                    // that separates this child from its left neighbour (if it has one).
                    if (childInd > 0 &&
                        result.removedItem->dynamicCompareToKey(signposts[childInd-1], itemKeyIndex) == 0 &&
                        result.lastLocation == LastLocation::LEAF) {
                        updateSignpost(childInd - 1);
                    }
                    break;

                case RemovalAction::STOLE_FROM_LEFT:
                    // Our child gained a new first item from its left sibling, so its
                    // separator signpost (childInd-1) changed.
                    result.action = RemovalAction::SIMPLE_REMOVAL;
                    if (childInd > 0) updateSignpost(childInd - 1);
                    break;

                case RemovalAction::STOLE_FROM_RIGHT:
                    // The right sibling lost its first item, so the separator between
                    // our child and that sibling (signpost[childInd]) changed.
                    result.action = RemovalAction::SIMPLE_REMOVAL;
                    updateSignpost(childInd);
                    break;

                case RemovalAction::MERGED_INTO_LEFT:
                    // Child merged into its left sibling — remove both the child and its
                    // separator signpost (the one pointing to this child, at childInd-1).
                    removeChildAt(childInd);
                    if (childInd - 1 < numSignposts) removeSignpostAt(childInd - 1);
                    break;

                case RemovalAction::MERGED_INTO_RIGHT:
                    // Child merged into its right sibling — remove the child and the
                    // signpost that pointed to the right sibling (now absorbed, at childInd).
                    removeChildAt(childInd);
                    if (childInd < numSignposts) removeSignpostAt(childInd);
                    break;
            }

            // Free only the pages we explicitly fetched at this level.
            // The dying child's page was already deallocated by merge(); don't touch it.
            if (leftChildPage  != (size_t)-1) bufferpool->freePage(leftChildPage);
            if (rightChildPage != (size_t)-1) bufferpool->freePage(rightChildPage);
            RemovalAction postAction = result.action;
            if (postAction != RemovalAction::MERGED_INTO_LEFT &&
                postAction != RemovalAction::MERGED_INTO_RIGHT) {
                bufferpool->freePage(childPage);
            }

            // Handle underfull condition
            if (checkUnderfull() && !isRoot()) {
                return handleUnderfull(result, leftSiblingHere, rightSiblingHere);
            }

            result.action = RemovalAction::SIMPLE_REMOVAL;
            result.lastLocation = LastLocation::INTERNAL;
            return result;
        }
        



        size_t overthrowRoot() {
            
            if (numChildren > 1) {
                throw std::runtime_error("Logic error: Trying to overthrow a root with more than one child");
            }
            size_t newRoot = children[0];
            removeChildAt(0);
            return newRoot;
        }




        /*
        Search for all items with the same key
        */
        ItemInterface* singleKeySearch(T findIt) {
            size_t childPageId = children[getChildIndexByKey(findIt)];
            ItemInterface* result = bufferpool->getNode(childPageId)->singleKeySearch(findIt);
            bufferpool->freePage(childPageId);
            return result;
        }




        // "inorder traversal" that prints the root half-way through iterating through subtrees
        void print(int depth) {
            for (int i = numChildren-1; i >= 0; i--)
            {
                bufferpool->getNode(children[i])->print(depth+1);
                bufferpool->freePage(children[i]);
                if (i == numChildren / 2) // Print this
                {
                    // Print this:
                    for (int j = 0; j < depth; j++)
                    {
                        cout << "                    ";
                    }
                    cout << "D" << depth << "-I" << "-@" << page << ":";
                    for (int j = 0; j < numSignposts; j++)
                    {

                        printKey(signposts[j]);
                        cout << ",";
                    }
                    cout << endl;
                }
            }
        }




        // Go hard left and rip down the linked list when you reach a leaf
        // TODO: might not want to use this on the disk version.
        void ripPrint(int depth) {
            size_t childPageId = children[0];
            bufferpool->getNode(childPageId)->ripPrint(depth+1);
            bufferpool->freePage(childPageId);
        }




        // POLYMORPHISM OBLIGATIONS
        void setNext(size_t newNext) {throw std::runtime_error("tried to do a linked list operation on an internal node");}
        void setPrev(size_t newPrev) {throw std::runtime_error("tried to do a linked list operation on an internal node");}
        BPNode<T, way>* getNext() {throw std::runtime_error("tried to do a linked list operation on an internal node");}
        void deserializeItems() {throw std::runtime_error("Tried to deserialize items on an internal node");}
};
    

#endif