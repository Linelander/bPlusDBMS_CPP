#include <cstddef>
#include<iostream>
#include "BPInternalNode.h"
#include "Item.h"
#include <string>
#include <vector>

#define NO_SPLIT NULL;

using namespace std;


// CONSTRUCTORS
template <typename T>
BPInternalNode<T>::BPInternalNode(int way, int keyIndex) {
    this->way = way;
    this->signCapacity = way-1;
    this->itemKeyIndex = keyIndex;

    // this->children.resize(1);
    // this->signposts.resize(signCapacity);
}

template <typename T>

BPInternalNode<T>::BPInternalNode(int way, int keyIndex, size_t nonstandardSize) {
    this->way = way;
    this->signCapacity = way-1;
    this->itemKeyIndex = keyIndex;

    // this->children.resize(1);
    // this->signposts.resize(signCapacity); // What?

    pageSize = nonstandardSize;
}
    
// METHODS


template <typename T>
bool BPInternalNode<T>::isRoot() {return rootBool;}


template <typename T>
void BPInternalNode<T>::makeRoot() {rootBool = true;}

template <typename T>
void BPInternalNode<T>::notRoot() {rootBool = false;}

template <typename T>
int BPInternalNode<T>::getWay()            {return this->way;}

template <typename T>
bool BPInternalNode<T>::isOverFull()       {return signposts.size() > signCapacity;}

template <typename T>
bool BPInternalNode<T>::isLeaf()           {return false;}

template <typename T>
int BPInternalNode<T>::getDepth(int depth) {return children[0]->getDepth(depth+1);}

template <typename T>
vector<BPNode<T>*>* BPInternalNode<T>::getChildren() {return &children;}


template <typename T>
int BPInternalNode<T>::numChildren() {
    return children.size();
}


template <typename T>
void BPInternalNode<T>::receiveChild(BPNode<T>* givenChild, int givenPost)
{
    auto newFrontChild = children.begin();
    children.insert(newFrontChild, givenChild);

    auto newFrontPost = signposts.begin();
    signposts.insert(newFrontPost, givenPost);
}



template <typename T>
void BPInternalNode<T>::giveChild(BPInternalNode* receiver) {
        // pop child
        BPNode<T>* popChild = children.back();
        children.pop_back();
        
        // pop signpost
        int popPost = *prev(signposts.end());
        signposts.pop_back();
        
        
        // give child and signpost
        receiver->receiveChild(popChild, popPost);

}





/*
This method will split our node, creating a new sibling
The new sibling will have an extra signpost. 
It is the responsibility of other methods to steal this signpost for the parent (see promote)
*/
template <typename T>
BPNode<T>* BPInternalNode<T>::split() {
    // redistribute children to a new node
    BPInternalNode* sibling = new BPInternalNode(way, itemKeyIndex, pageSize);
    while (sibling->numChildren() != this->children.size()+1 && sibling->numChildren() != this->children.size()) {
        giveChild(sibling);
    }

    return sibling;
}

template <typename T>
T BPInternalNode<T>::viewSign1() {
    auto front = signposts.begin();
    return *front;
}

template <typename T>
T BPInternalNode<T>::getSign1()
{
    auto front = signposts.begin();
    int result = *front;
    signposts.erase(front);
    return result;
}


/* Handles adding new children created by splits to the children list.
    This is also where keys are stolen during splits
*/
template <typename T>
void BPInternalNode<T>::sortedInsert(BPNode<T>* newChild) {
    
    // Insert signpost:
    int newSign{};
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
        else if (*currSign > newSign) // should never be equal - no dupes
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
        else if ((*currChild)->viewSign1() > newSign) // should never be equal - no dupes
        {
            children.insert(currChild, newChild);
            break;
        }
        currChild++;
    }
}

template <typename T>
void BPInternalNode<T>::becomeInternalRoot(vector<BPNode<T>*> newChildren)
{
    children.push_back(newChildren.front());
    sortedInsert(newChildren.back());   
    makeRoot();
}


/* The method parents use to steal/copy keys from newborn children
    Return value of null: no split occured
*/
template <typename T>
BPNode<T>* BPInternalNode<T>::promote(BPNode<T>* rep) {
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





template <typename T>
void BPInternalNode<T>::becomeFirstInternalRoot(vector<BPLeaf<T>*> newChildren) {
    children.push_back(newChildren.front());
    children.push_back(newChildren.back());
    signposts.push_back(newChildren.back()->viewSign1());
}




/* When inserting on internal nodes that are children, add the result of insertion to the children list IF its pointer is different from the one you inserted on.
    After a recursive call resulting in a split, promote handles the copying/stealing of the new child's key (whichever is needed)
    TODO: what happens when promote returns null?
*/ 
template <typename T>
BPNode<T>* BPInternalNode<T>::insert(Item newItem) {
    BPNode<T>* result{};

    int finalChild = signposts.size();
    int finalPost = signposts.size()-1;
    int penultimateChild = signposts.size()-1;
    // Those should be accessible by the entire class. Maybe even fields for testing purposes ^^^^

    for (int i = 0; i < signposts.size(); i++) 
    {
        if (i == signposts.size()-1)                                                            // BACK POST
        {
            if (newItem.getPrimaryKey() < signposts[finalPost])
            {
                result = children[penultimateChild]->insert(newItem); // PENULTIMATE CHILD
                if (result == NULL) { // no split
                    return NULL;
                }
                else {
                    return promote(result); // if split
                }
            }
            else if (newItem.getPrimaryKey() >= signposts[finalPost]) {
                result = children[finalChild]->insert(newItem); // BACK CHILD
                if (result == NULL) { // no split
                    return NULL;
                }
                else {
                    return promote(result); // if split
                }
            }
        }
        else if (i == 0 && newItem.getPrimaryKey() < signposts[i])
        {
            result = children[0]->insert(newItem); // FRONT CHILD
            if (result == NULL) { // no split
                return NULL;
            }
            else {
                return promote(result); // if split
            }
        }
        else if (newItem.getPrimaryKey() >= signposts[i-1] && newItem.getPrimaryKey() < signposts[i]) {     // MIDDLE CHILD
            result = children[i]->insert(newItem);
            if (result == NULL) { // no split
                return NULL;
            }
            else {
                return promote(result); // if split
            }
        }
    }
}


template <typename T>
int BPInternalNode<T>:: remove(int deleteIt) {
    return -99; // placeholder for compiler
}



template <typename T>
vector<Item> BPInternalNode<T>::search(int findIt) {

}


// "inorder traversal" that prints the root half-way through iterating through subtrees
template <typename T>
void BPInternalNode<T>::print(int depth) {
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
                cout << signposts[j];
                cout << ",";
            }
            cout << endl;
        }
    }
}