#include <cstddef>
#include <cstdio>
#include<iostream>
#include <stdexcept>
#include "BPNode.h"


class ItemInterface;

template<typename T, int way> class BPLeaf;

using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

template<typename T, int way>
class BPInternalNode : public BPNode<T, way> {
    private:
        int itemKeyIndex; // "index" of column we're creating an index on
        bool rootBool{false};
        int pageSize = 4096;
        int signCapacity{};
        int numSignposts{};
        int numChildren{};
        
        // Disk
        size_t pageIndex;
        Freelist* freelist;


        bool isOverFull() {return numSignposts > signCapacity;}

        std::array<BPNode<T, way>*, way+1> children{};
        std::array<T, way> signposts;
        // NOTE: using 1 dummy slot at the end of each array for splitting logic.


    public:
        // CONSTRUCTORS / DEST.
        BPInternalNode(int keyIndex, Freelist* fList) : itemKeyIndex(keyIndex) {
            this->signCapacity = way-1;
            freelist = fList;
            pageIndex = freelist->allocate();
        }


        BPInternalNode(int keyIndex, size_t nonstandardSize, Freelist* fList) : itemKeyIndex(keyIndex), pageSize(nonstandardSize) {
            this->signCapacity = way-1;
            freelist = fList;
            pageIndex = freelist->allocate();
        }


        ~BPInternalNode() {
            for (int i = 0; i < numChildren; i++) {
                delete children[i];
            }
        }
            

        // METHODS
        bool isRoot() {return rootBool;}
        void makeRoot() {rootBool = true;}
        void notRoot() {rootBool = false;}
        bool isLeaf()           {return false;}


        void printKey(int key) {
            cout << key;
        }


        void printKey(const AttributeType& attr) {
            cout << attr.data();
        }


        int getNumChildren() {
            return numChildren;
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
        void receiveChild(BPNode<T, way>* givenChild, T givenPost) {
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
            
            BPNode<T, way>* popChild = children[numChildren - 1];
            children[numChildren - 1] = nullptr;
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
        BPNode<T, way>* split() {
            // redistribute children to a new node
            BPInternalNode* sibling = new BPInternalNode(itemKeyIndex, pageSize);
            while (sibling->getNumChildren() != this->numChildren+1 && sibling->getNumChildren() != this->numChildren) {
                giveChild(sibling);
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


        void insertChild(BPNode<T, way>* child, int pos) {
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
            
            children[numChildren - 1] = nullptr;
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
        void sortedInsert(BPNode<T, way>* newChild) {
            // Insert signpost:
            T newSign{};
            if (newChild->isLeaf())
            {
                newSign = newChild->viewSign1(); // leaf split: adopt key from child but do not steal
            }
            else {
                newSign = newChild->getSign1(); // internal split: steal key from child
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
            int j = 0;
            while (true)
            {
                if (j == numChildren) {
                    insertChild(newChild, numChildren);
                    break;
                }
                else if (children[j]->viewSign1() > newSign)
                {
                    insertChild(newChild, j);
                    break;
                }
                j++;
            }
        }

        void becomeInternalRoot(std::array<BPNode<T, way>*, 2> newChildren) {
            insertChild(newChildren[0], 0); // FLAG
            sortedInsert(newChildren[1]);
            makeRoot();
        }


        /* The method parents use to steal/copy keys from newborn children
            Return value of null: no split occured
        */
        BPNode<T, way>* promote(BPNode<T, way>* rep) {
            sortedInsert(rep);

            BPNode<T, way>* splitResult = NULL;
            if (isOverFull())
            {
                splitResult = split();

                if (isRoot()) {
                    BPInternalNode* newRoot = new BPInternalNode(itemKeyIndex, pageSize);
                    std::array<BPNode<T, way>*, 2> rootChildren = {this, splitResult};
                    // first keys are stolen by a call to sorted insert inside this method
                    newRoot->becomeInternalRoot(rootChildren);
                    this->notRoot();
                    splitResult = newRoot;
                    return splitResult;
                }

                return splitResult;
            }

            // Case 3: We do not split. Null represents no split.
            return NULL;
        }


        void becomeFirstInternalRoot(std::array<BPLeaf<T, way>*, 2> newChildren) {
            insertChild(newChildren[0], numChildren);
            insertChild(newChildren[1], numChildren);
            insertSignpost(newChildren[1]->viewSign1(), numSignposts);
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
        
        
        /* When inserting on internal nodes that are children, add the result of insertion to the children list IF its pointer is different from the one you inserted on.
        After a recursive call resulting in a split, promote handles the copying/stealing of the new child's key (whichever is needed)            
        */ 
        BPNode<T, way>* insert(ItemInterface* newItem) {
            BPNode<T, way>* result{};
            T newItemkey = any_cast<T>(newItem->dynamicGetKeyByIndex(itemKeyIndex));
            result = children[getChildIndexByKey(newItemkey)]->insert(newItem);
            if (result == NULL) {
                return NULL;
            }
            return promote(result);
        }
        

        // TOD maybe this should check signposts instead. thinking about 2-3+ trees.
        bool checkUnderfull() {
            return (numChildren < children.size() / 2);
            // return (numSignposts < children.size() / 2);
        }

        
        bool isWealthy() {
            return (numChildren == (children.size() / 2) + 1);
        }
        

        BPNode<T, way>* backSteal() {
            BPNode<T, way>* result = children[numChildren-1];
            removeChildAt(numChildren-1);
            if (numSignposts > 0) {
                removeSignpostAt(numSignposts-1);
            }
            return result;
        }


        BPNode<T, way>* frontSteal() {
            BPNode<T, way>* result = children[0];
            removeChildAt(0);
            if (numSignposts > 0) {
                removeSignpostAt(0);
            }
            return result;
        }


        // Get the leftmost value from this subtree
        T getHardLeft() {
            return children[0]->getHardLeft();
        }


        // Look at hard left value in children to generate signposts
        void generateSignposts() {
            while (numSignposts > 0)
            {
                removeSignpostAt(0);
            }
            
            for (int i = 1; i < numChildren; i++)
            {
                insertSignpost(children[i]->getHardLeft(), i-1);
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
                BPNode<T, way>* stolen = leftSiblingHere->backSteal();
                insertChild(stolen, 0);
                
                modifyResult.action = RemovalAction::STOLE_FROM_LEFT;
                structureChanged = true;
                cout << "---- LEFT STEAL internal ----" << endl;
            }

            // STEAL FROM RIGHT
            else if (rightSiblingHere != nullptr && rightSiblingHere->isWealthy()) {
                BPNode<T, way>* stolen = rightSiblingHere->frontSteal();
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
                cout << "---- LEFT MERGE internal ----" << endl;
            }

            // MERGE WITH RIGHT
            else if (rightSiblingHere != nullptr) {
                rightSiblingHere->mergeRightHere(this);
                modifyResult.action = RemovalAction::MERGED_INTO_RIGHT;
                structureChanged = true;
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
            BPNode<T, way>* leftSiblingDown = nullptr;
            BPNode<T, way>* rightSiblingDown = nullptr;
            if (leftChildInd >= 0) {
                leftSiblingDown = children[leftChildInd];
            }
            if (rightChildInd < numChildren) {
                rightSiblingDown = children[rightChildInd];
            }

            // actual removal call
            RemovalResult<T> result = children[childInd]->remove(deleteIt, leftSiblingDown, rightSiblingDown);
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
                    delete children[childInd];    
                    removeChildAt(childInd);
                    needsSignpostRegeneration = true;
                    break;

                case RemovalAction::MERGED_INTO_RIGHT:
                    delete children[childInd];
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


        
        BPNode<T, way>* overthrowRoot() {
            
            if (numChildren > 1) {
                throw std::runtime_error("Logic error: Trying to overthrow a root with more than one child");
            }
            BPNode<T, way>* newRoot = children[0];
            removeChildAt(0);
            return newRoot;
        }



        /*
        Search for all items with the same key
        */
        ItemInterface* singleKeySearch(T findIt) {
            return children[getChildIndexByKey(findIt)]->singleKeySearch(findIt);
        }



        // "inorder traversal" that prints the root half-way through iterating through subtrees
        void print(int depth) {
            for (int i = numChildren-1; i >= 0; i--)
            {
                children[i]->print(depth+1);

                if (i == numChildren / 2) // Print this
                {
                    // Print this:
                    for (int j = 0; j < depth; j++)
                    {
                        cout << "                    ";
                    }
                    cout << "D" << depth << "-I:";
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
        void ripPrint(int depth) {
            children[0]->ripPrint(depth+1);
        }


        // POLYMORPHISM OBLIGATIONS
        void setNext(BPNode<T, way>* newNext) {throw std::runtime_error("tried to do a linked list operation on an internal node");}
        void setPrev(BPNode<T, way>* newPrev) {throw std::runtime_error("tried to do a linked list operation on an internal node");}
        BPNode<T, way>* getNext() {throw std::runtime_error("tried to do a linked list operation on an internal node");}
};
    

#endif