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

        // vector<T> signposts; // MUST LIMIT THE SIZE OF THIS LIST
        // vector<BPNode<T>*> children{};

        std::array<BPNode<T>*, way> children{};
        std::array<T, way-1> signposts{};              // way-1 max signposts


    public:
        // CONSTRUCTORS
        BPInternalNode(int keyIndex) {
            this->way = way;
            this->signCapacity = way-1;
            this->itemKeyIndex = keyIndex;
        }


        BPInternalNode(int keyIndex, size_t nonstandardSize) {
            this->way = way;
            this->signCapacity = way-1;
            this->itemKeyIndex = keyIndex;
            pageSize = nonstandardSize;
        }
            
        // METHODS
        bool isRoot() {return rootBool;}
        void makeRoot() {rootBool = true;}
        void notRoot() {rootBool = false;}
        int getWay()            {return this->way;}
        bool isOverFull()       {return signposts.size() > signCapacity;}
        bool isLeaf()           {return false;}
        int getDepth(int depth) {return children[0]->getDepth(depth+1);}
        vector<BPNode<T>*>* getChildren() {return &children;}



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
            // auto newFrontChild = children.begin();
            // children.insert(newFrontChild, givenChild);
            children.insertChild(givenChild, 0);
            numChildren++;


            // auto newFrontPost = signposts.begin();
            // signposts.insert(newFrontPost, givenPost);
            signposts.insertSignpost(givenPost, 0);
            numSignposts++;
        }



        // void giveChild(BPInternalNode* receiver) {
        //         // pop child
        //         BPNode<T>* popChild = children.back();
        //         children.pop_back();
                
        //         // pop signpost
        //         T popPost = *prev(signposts.end());
        //         signposts.pop_back();
                
        //         // give child and signpost
        //         receiver->receiveChild(popChild, popPost);
        // }

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
            BPInternalNode* sibling = new BPInternalNode(way, itemKeyIndex, pageSize);
            while (sibling->numChildren() != this->children.size()+1 && sibling->numChildren() != this->children.size()) {
                giveChild(sibling);
            }

            return sibling;
        }

        void insertSignpost(const T& key, int pos) {
            for (int i = numSignposts; i > pos; i--) {
                signposts[i] = signposts[i-1];
            }
            signposts[pos] = key;
            numSignposts++;
        }

        void insertChild(BPNode<T>* child, int pos) {
            for (int i = numChildren; i > pos; i--) {
                children[i] = children[i-1];
            }
            children[pos] = child;
            numChildren++;
        }

        void removeChildAt(int position) {
            if (position < 0 || position >= numChildren) {
                throw std::out_of_range("Child position out of range");
            }
            
            for (int i = position; i < numChildren - 1; i++) {
                children[i] = children[i + 1];
            }
            
            children[numChildren - 1] = nullptr;
            numChildren--;
        }
    
        // Remove signpost at specific position
        void removeSignpostAt(int position) {
            if (position < 0 || position >= numSignposts) {
                throw std::out_of_range("Signpost position out of range");
            }
            
            for (int i = position; i < numSignposts - 1; i++) {
                signposts[i] = signposts[i + 1];
            }
            
            signposts[numSignposts - 1] = T{};
            numSignposts--;
        }


        T viewSign1() {
            return *signposts[0];
        }


        T getSign1()
        {
            T result = *signposts[0];
            removeSignpostAt(0);
            numSignposts--;
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
                newSign = newChild->viewSign1();  // leaf split: adopt key from child but do not steal
            }
            else {
                newSign = newChild->getSign1(); // internal split: steal key from child
            }

            auto currSign = signposts.begin();
            while (true)
            {
                if (currSign == signposts.end())
                {
                    signposts.push_back(newSign);
                    break;
                }
                else if (*currSign > newSign)
                {
                    signposts.insert(currSign, newSign);
                    break;
                }
                currSign++;
            }

            // insert child
            auto currChild = children.begin();
            while (true)
            {
                if (currChild == children.end()) {
                    children.push_back(newChild);
                    break;
                }
                else if ((*currChild)->viewSign1() > newSign)
                {
                    children.insert(currChild, newChild);
                    break;
                }
                currChild++;
            }
        }

        void becomeInternalRoot(vector<BPNode<T>*> newChildren)
        {
            children.push_back(newChildren.front());
            sortedInsert(newChildren.back());   
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
                    BPInternalNode* newRoot = new BPInternalNode(way, itemKeyIndex, pageSize);
                    vector<BPNode<T>*> rootChildren = {this, splitResult};
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



        void becomeFirstInternalRoot(vector<BPLeaf<T, way>*> newChildren) {
            children.push_back(newChildren.front());
            children.push_back(newChildren.back());
            signposts.push_back(newChildren.back()->viewSign1());
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


        int remove(int deleteIt) {
            return -99; // placeholder for compiler
        }


        /*
        Search for all items with the same key
        */
        vector<ItemInterface*> singleKeySearch(T findIt) {
            return children[getChildIndexByKey(findIt)]->singleKeySearch(findIt);
        }


        // "inorder traversal" that prints the root half-way through iterating through subtrees
        void print(int depth) {
            for (int i = children.size()-1; i >= 0; i--)
            {
                children[i]->print(depth+1);


                if (i == children.size() / 2) // Print this
                {
                    // Print this:
                    for (int j = 0; j < depth; j++)
                    {
                        cout << "                    ";
                    }
                    cout << "D" << depth << "-I:";
                    for (int j = 0; j < signposts.size(); j++)
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