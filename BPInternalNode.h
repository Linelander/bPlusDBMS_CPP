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
        
        static const size_t INVALID_PAGE_ID = -1;
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
            vector<uint8_t> bytes;

            lseek(fd, offset, SEEK_SET);

            checkRW(write(fd, bytes.data(), bytes.size()));

            // Bufferpool calls delete. should delete do anything extra?
        }

        
        // DISK
        NodePage<T, way> getPage(){return page;}

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
            BPInternalNode* siblingNode = new BPInternalNode(itemKeyIndex, bufferpool, pageSize);
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
            This is also where keys are stolen during splits
        */
        void sortedInsert(size_t newChild) {
            // Insert signpost:
            T newSign{};
            if (bufferpool->getNode(newChild)->isLeafFn())
            {
                newSign = bufferpool->getNode(newChild)->viewSign1(); // leaf split: adopt key from child but do not steal
            }
            else {
                newSign = bufferpool->getNode(newChild)->getSign1(); // internal split: steal key from child
            }


            int i = 0;
            while (true)
            {
                if (i == numSignposts)
                {
                    insertSignpost(newSign, numSignposts);
                    break;
                }
                else if (signposts[i] > newSign)
                {
                    insertSignpost(newSign, i);
                    break;
                }
                i++;
            }

            // insert child
            /* 
                TODO: this can be refactored to go off of signposts instead of viewSign1,
                meaning we won't have to do disk accesses for every comparison once disk is implemented
            */
            int j = 0;
            while (true)
            {
                if (j == numChildren) {
                    insertChild(newChild, numChildren);
                    break;
                }
                else if (bufferpool->getNode(children[j])->viewSign1() > newSign)
                {
                    insertChild(newChild, j); // TODO refactor
                    break;
                }
                j++;
            }
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
                    BPInternalNode* newRoot = new BPInternalNode<T, way>(itemKeyIndex, bufferpool, pageSize);
                    std::array<size_t, 2> rootChildren = {this, splitResult};
                    // first keys are stolen by a call to sorted insert inside this method
                    newRoot->becomeInternalRoot(rootChildren);
                    this->notRoot();
                    splitResult = newRoot;
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


        // Look at hard left value in children to generate signposts
        // TODO: replace this with something more targeted.
        void generateSignposts() {
            while (numSignposts > 0)
            {
                removeSignpostAt(0);
            }
            
            for (int i = 1; i < numChildren; i++)
            {
                insertSignpost(bufferpool->getNode(children[i])->getHardLeft(), i-1);
            }

            // Freeing all
            for (int i = 0; i < numChildren; i++) {
                bufferpool->freePage(children[i]);
            }
        }


        void mergeLeftHere(BPNode<T, way>* dyingNode) {
            while (dyingNode->getNumChildren() > 0) {
                insertChild(dyingNode->frontSteal(), numChildren);
            }
            generateSignposts();
        }


        void mergeRightHere(BPNode<T, way>* dyingNode) {
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
                
                modifyResult.action = RemovalAction::STOLE_FROM_LEFT;
                structureChanged = true;
                cout << "---- LEFT STEAL internal ----" << endl;
            }

            // STEAL FROM RIGHT
            else if (rightSiblingHere != nullptr && rightSiblingHere->isWealthy()) {
                size_t stolen = rightSiblingHere->frontSteal();
                insertChild(stolen, numChildren);
                
                modifyResult.action = RemovalAction::STOLE_FROM_RIGHT;
                structureChanged = true;
                cout << "---- RIGHT STEAL internal ----" << endl;
            }

            // MERGE WITH LEFT
            else if (leftSiblingHere != nullptr) {
                leftSiblingHere->mergeLeftHere(this);
                modifyResult.action = RemovalAction::MERGED_INTO_LEFT;
                structureChanged = true;
                bufferpool->deallocate(pageOffset); // maybe a separate call for destructions?
                cout << "---- LEFT MERGE internal ----" << endl;
            }

            // MERGE WITH RIGHT
            else if (rightSiblingHere != nullptr) {
                rightSiblingHere->mergeRightHere(this);
                modifyResult.action = RemovalAction::MERGED_INTO_RIGHT;
                structureChanged = true;
                bufferpool->deallocate(pageOffset); // maybe a separate call for destructions?
                cout << "---- RIGHT MERGE internal ----" << endl;
            }
            
            // Only regenerate signposts if structure actually changed
            if (structureChanged) {
                generateSignposts();
            }
            
            modifyResult.lastLocation = LastLocation::INTERNAL;
            return modifyResult;
        }



        RemovalResult<T> remove(T deleteIt, BPNode<T, way>* leftSiblingHere, BPNode<T, way>* rightSiblingHere) {
            
            int childInd = getChildIndexByKey(deleteIt);
            int leftChildInd = childInd-1;
            int rightChildInd = childInd+1;
            
            // We're above the target leaf. Grab references to its eligible wealthy siblings if it's poor.
            BPNode<T, way>* leftSiblingDown;
            BPNode<T, way>* rightSiblingDown;
            if (leftChildInd >= 0) {
                leftSiblingDown = bufferpool->getNode(children[leftChildInd]);
            }
            if (rightChildInd < numChildren) {
                rightSiblingDown = bufferpool->getNode(children[rightChildInd]);
            }

            // actual removal call
            RemovalResult<T> result = bufferpool->getNode(children[childInd])->remove(deleteIt, leftSiblingDown, rightSiblingDown);
            RemovalAction action = result.action;
            
            bool needsSignpostRegeneration = false;
            
            /*
                SWITCH BASED ON WHAT HAPPENED AT THE CHILD
            */
            switch (action) {
                case RemovalAction::DEFAULT:
                    throw std::runtime_error("Removal action should not be default at parent-child relationship management switch statement.");

                case RemovalAction::SIMPLE_REMOVAL:
                    // Only regenerate signposts if we're dealing with a key that might affect signposts
                    if (childInd > 0 && 
                        result.removedItem->dynamicCompareToKey(signposts[childInd-1], itemKeyIndex) == 0 && 
                        result.lastLocation == LastLocation::LEAF) {
                        needsSignpostRegeneration = true;
                        cout << "---- SIMPLE REMOVE internal ----" << endl;
                    }
                    break;

                case RemovalAction::STOLE_FROM_LEFT:
                    result.action = RemovalAction::SIMPLE_REMOVAL;
                    needsSignpostRegeneration = true; // Always need to regenerate after stealing
                    break;

                case RemovalAction::STOLE_FROM_RIGHT:
                    result.action = RemovalAction::SIMPLE_REMOVAL;
                    needsSignpostRegeneration = true; // Always need to regenerate after stealing
                    break;

                case RemovalAction::MERGED_INTO_LEFT:
                    removeChildAt(childInd);
                    needsSignpostRegeneration = true;
                    break;

                case RemovalAction::MERGED_INTO_RIGHT:
                    removeChildAt(childInd);
                    needsSignpostRegeneration = true;
                    break;
            }

            // Regenerate signposts once after all structural changes
            if (needsSignpostRegeneration) {
                generateSignposts();
            }

            // Handle underfull condition
            if (checkUnderfull() && !isRoot()) {
                // Don't regenerate here - let handleUnderfull do it if needed
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
                    cout << "D" << depth << "-I" << "-@" << pageOffset << ":";
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
        void setNext(BPNode<T, way>* newNext) {throw std::runtime_error("tried to do a linked list operation on an internal node");}
        void setPrev(BPNode<T, way>* newPrev) {throw std::runtime_error("tried to do a linked list operation on an internal node");}
        BPNode<T, way>* getNext() {throw std::runtime_error("tried to do a linked list operation on an internal node");}
};
    

#endif