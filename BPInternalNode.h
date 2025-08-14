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

        bool isOverFull() {return numSignposts > signCapacity;}

        std::array<BPNode<T, way>*, way+1> children{};
        std::array<T, way> signposts;
        // NOTE: using 1 dummy slot at the end of each array for splitting logic.


    public:
        // CONSTRUCTORS / DEST.
        BPInternalNode(int keyIndex) {
            this->signCapacity = way-1;
            this->itemKeyIndex = keyIndex;
        }


        BPInternalNode(int keyIndex, size_t nonstandardSize) {
            this->signCapacity = way-1;
            this->itemKeyIndex = keyIndex;
            pageSize = nonstandardSize;
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
        int getDepth(int depth) {return children[0]->getDepth(depth+1);}


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
                    std::array<BPNode<T, way>*, 2> rootChildren = {this, splitResult};                      // TODO array
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

        
        
        // TODO: double check this
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
        


        void childOnlySortedInsert() {

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
        
        

        // int getSignIndexByKey(T key) {
        //     if (key < signposts[0]) {
        //         return -1;
        //     }
        //     int i = 0;
        //     T find = signposts[i];
        //     while (i < numSignposts && find < key) {
        //         i++;
        //         find = signposts[i];
        //     }
        //     return i;
        // }

        int getSignIndexByKey(T key) {
            if (key < signposts[0]) {
                return -1;
            }
            int i = 0;
            while (i < numSignposts && signposts[i] < key) {
                i++;
            }
            return i;
        }
                


        // TODO: validate this. suspicious use of viewSign1.
        // This should only be used above leaves. Maybe change the name to reflect that. Or just get rid of this.
        // Hard left could be used for the same thing we're using viewSign1 for.
        void updateSignpost(T signToChange) {
            T newKey = children[getChildIndexByKey(signToChange)]->viewSign1();
            if (signToChange == newKey)
            {
                // TODO exception
            }
            signposts[getSignIndexByKey(signToChange)] = newKey;
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
            // if (!isWealthy())
            // {
            //     throw std::runtime_error("Tried to steal from nonwealthy sibling");
            // }
            BPNode<T, way>* result = children[numChildren-1];
            removeChildAt(numChildren-1);
            if (numSignposts > 0) {
                removeSignpostAt(numSignposts-1);
            }
            return result;
        }

        BPNode<T, way>* frontSteal() {
            // if (!isWealthy())
            // {
            //     throw std::runtime_error("Tried to steal from nonwealthy sibling");
            // }
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
            for (int i = 1; i < numChildren; i++)
            {
                signposts[i-1] = children[i]->getHardLeft();
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
        


        // THIS CONTROLS WHAT INTERNALS DO FOR THEIR DELETIONS
        RemovalResult<T> handleUnderfull(RemovalResult<T> modifyResult, BPNode<T, way>* leftSiblingHere, BPNode<T, way>* rightSiblingHere) {
            if (leftSiblingHere == rightSiblingHere){
                throw std::runtime_error("Left and right siblings identical during internal underfull handling. Likely both null (should be impossible).");
            }
            


            // STEAL FROM LEFT
            if (leftSiblingHere != nullptr && leftSiblingHere->isWealthy()) {
                insertSignpost(children[0]->getHardLeft(), 0);
                BPNode<T, way>* stolen = leftSiblingHere->backSteal();
                insertChild(stolen, 0);

                modifyResult.rightSubtreeMin = stolen->getHardLeft();
                modifyResult.action = RemovalAction::STOLE_FROM_LEFT;
                // then in the parent:
                /*
                Find the signpost pointing to THIS
                Replace it with stolenChildMinKey
                */
                cout << "---- LEFT STEAL internal ----" << endl;
            }


            // STEAL FROM RIGHT
            else if (rightSiblingHere != nullptr && rightSiblingHere->isWealthy()) {
                // Steal lowest from right sibling and add its sign
                BPNode<T, way>* stolen = rightSiblingHere->frontSteal();
                insertSignpost(stolen->getHardLeft(), numSignposts); // WARN is this an appropriate use of VS1?
                insertChild(stolen, numChildren);

                modifyResult.rightSubtreeMin = rightSiblingHere->getHardLeft();
                modifyResult.action = RemovalAction::STOLE_FROM_RIGHT;
                // then in the parent:
                /*
                Find the signpost pointing to RIGHT SIBLING
                Replace it with stolenChildMinKey
                */
                cout << "---- RIGHT STEAL internal ----" << endl;
            }


            // MERGE WITH LEFT
            else if (leftSiblingHere != nullptr) {
                leftSiblingHere->mergeLeftHere(this);
                modifyResult.action = RemovalAction::MERGED_INTO_LEFT;
                cout << "---- LEFT MERGE internal ----" << endl;
            }


            // MERGE WITH RIGHT
            else if (rightSiblingHere != nullptr) {
                rightSiblingHere->mergeRightHere(this);
                modifyResult.action = RemovalAction::MERGED_INTO_RIGHT;
                cout << "---- RIGHT MERGE internal ----" << endl;
            }

            return modifyResult;
        }








        RemovalResult<T> remove(T deleteIt, BPNode<T, way>* leftSiblingHere, BPNode<T, way>* rightSiblingHere) {
            
            int childInd = getChildIndexByKey(deleteIt);
            int leftChildInd = childInd-1;
            int rightChildInd = childInd+1;
            
            int childSignIndex = getSignIndexByKey(deleteIt); // what if it's negative 1?
            int rightSignIndex = childSignIndex + 1;
            
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
            RemovalResult<T> result = children[childInd]->remove(deleteIt, leftSiblingDown, rightSiblingDown); // leaf will use linked list to get right sibling itself.
            RemovalAction action  = result.action;

            // print(0);
            
            /*
                                THIS SWITCH STATEMENT CONTROLS WHAT THE PARENT DOES
                                SWITCH BASED ON WHAT HAPPENED AT THE CHILD
            */
            switch (action) {
                case RemovalAction::DEFAULT:
                    throw std::runtime_error("Removal action should not be default at parent-child relationship management switch statement.");


                case RemovalAction::SIMPLE_REMOVAL: // Easy case - return immediately and don't check underfull.
                    if (childInd != 0 
                        && result.removedItem->compareToKeyByIndex(signposts[childSignIndex], itemKeyIndex) == 0
                        && result.lastLocation == LastLocation::LEAF) {
                        updateSignpost(signposts[childSignIndex]);
                        cout << "---- SIMPLE REMOVE internal ----" << endl;
                    }
                    return result; // Don't need to change the action here.


                case RemovalAction::STOLE_FROM_LEFT:
                result.action = RemovalAction::SIMPLE_REMOVAL; // says we're done doing surgery...
                    if (result.lastLocation == LastLocation::LEAF) {
                        updateSignpost(signposts[childSignIndex]); // for leaves only?
                        return result; // ???
                    }
                    signposts[childSignIndex] = *result.rightSubtreeMin; // TODO TODO what happens to this value after this? 
                    result.rightSubtreeMin.reset(); // just reset it?
                    return result;


                case RemovalAction::STOLE_FROM_RIGHT:
                    result.action = RemovalAction::SIMPLE_REMOVAL; // says we're done doing surgery.
                    if (result.lastLocation == LastLocation::LEAF) {
                        signposts[rightSignIndex] = children[rightChildInd]->getHardLeft(); // for leaves only?
                        return result; // ???
                    }
                    signposts[rightSignIndex] = *result.rightSubtreeMin; // change the name of this to right subtree min
                    result.rightSubtreeMin.reset();
                    return result;


                // TODO: deal with edge cases where there's only one signpost. Decide if we will measure fullness based on signposts or children.

                // The next two cases break because something got deleted. We need to check for underfull.
                case RemovalAction::MERGED_INTO_LEFT: // Delete victim and victim pointing signpost
                    removeSignpostAt(childInd-1);
                    removeChildAt(childInd);
                    break;


                case RemovalAction::MERGED_INTO_RIGHT:
                    if (numSignposts > 1) { // Delete victim and right of victim pointing signpost
                        removeSignpostAt(rightSignIndex);
                    }
                    removeChildAt(childInd);
                    break;
            }


            if (checkUnderfull() && !isRoot()) {
                return handleUnderfull(result, leftSiblingHere, rightSiblingHere);
            }
            result.action = RemovalAction::SIMPLE_REMOVAL;
            return result;

            // So roots can have 0 signs.
            // When this happens, root's one child must become root
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
};
    

#endif