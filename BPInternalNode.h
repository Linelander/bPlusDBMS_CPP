#include<iostream>
#include "BPNode.h"

class ItemInterface;

template<typename T, int way> class BPLeaf;

using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

template<typename T, int way>
class BPInternalNode : public BPNode<T> {
    private:
        int itemKeyIndex; // "index" of column we're creating an index on
        bool rootBool{false};
        int pageSize = 4096;
        int signCapacity{};
        int numSignposts{};
        int numChildren{};

        bool isOverFull() {return numSignposts > signCapacity;}

        std::array<BPNode<T>*, way+1> children{};
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

        // Stacks a child on the front of the children array
        void receiveChild(BPNode<T>* givenChild, T givenPost)
        {
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
            
            BPNode<T>* popChild = children[numChildren - 1];
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
        BPNode<T>* split() {
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

        void insertChild(BPNode<T>* child, int pos) {
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


        T getSign1()
        {
            T result = signposts[0];
            removeSignpostAt(0);
            return result;
        }


        /* Handles adding new children created by splits to the children list.
            This is also where keys are stolen during splits
        */
        void sortedInsert(BPNode<T>* newChild) {
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

        void becomeInternalRoot(std::array<BPNode<T>*, 2> newChildren)                                 // TODO array
        {
            insertChild(newChildren[0], 0); // FLAG
            sortedInsert(newChildren[1]);
            makeRoot();
        }


        /* The method parents use to steal/copy keys from newborn children
            Return value of null: no split occured
        */
        BPNode<T>* promote(BPNode<T>* rep) {
            sortedInsert(rep);

            BPNode<T>* splitResult = NULL;
            if (isOverFull())
            {
                splitResult = split();

                if (isRoot()) {
                    BPInternalNode* newRoot = new BPInternalNode(itemKeyIndex, pageSize);
                    std::array<BPNode<T>*, 2> rootChildren = {this, splitResult};                      // TODO array
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
        BPNode<T>* insert(ItemInterface* newItem) {
            BPNode<T>* result{};
            T newItemkey = any_cast<T>(newItem->dynamicGetKeyByIndex(itemKeyIndex));
            result = children[getChildIndexByKey(newItemkey)]->insert(newItem);
            if (result == NULL) {
                return NULL;
            }
            return promote(result);
        }


        ItemInterface* remove(T deleteIt) {
            int childInd = getChildIndexByKey(deleteIt);
            int leftChildInd = childInd-1;
            int rightChildInd = childInd+1;
            
            
            // We're above the target leaf. Grab references to its eligible wealthy siblings if it's poor.
            BPLeaf<T, way>* leftSibling = nullptr;

            if (children[childInd].isLeaf()) {
                if (leftChildInd >= 0) {
                    leftSibling = children[leftChildInd];
                }
                
                /*
                Cases for leaves to handle:
                - Wealthy leaf (easy)
                - Poor leaf with wealthy sibling(s)
                - Poor leaf with no wealthy sibling(s)
                */
                if (children[childInd]->isWealthy()) {
                    // remember, before returning, this node needs to make sure it's not underfull.
                    return children.remove(deleteIt);
                }
                else {
                    // remember, before returning, this node needs to make sure it's not underfull.
                    return children.remove(deleteIt, leftSibling); // leaf will use linked list to get right sibling itself.
                }

            }
            else { // Child is internal.
                // remember, before returning, this node needs to make sure it's not underfull.
                return children[childInd]->remove(deleteIt);
            }




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