#include <cstddef>
#include<iostream>
#include "BPInternalNode.h"
#include "Item.h"
#include <string>

using namespace std;

// FIELDS
// bool rootBool{false};
// size_t pageSize{4096};
// int way{};          // max no. of children
// int signCapacity{}; // max no. of signposts
// vector<int> signposts{};
// vector<BPNode*> children{};

// CONSTRUCTORS
BPInternalNode::BPInternalNode(int way) {
    this->way = way;
    this->signCapacity = way-1;
}

BPInternalNode::BPInternalNode(int way, size_t nonstandardSize) {
    this->way = way;
    this->signCapacity = way+1;
    pageSize = nonstandardSize;
}
    
// METHODS
bool BPInternalNode::isRoot() {return rootBool;}
void BPInternalNode::makeRoot() {rootBool = true;}
void BPInternalNode::notRoot() {rootBool = false;}
int BPInternalNode::getWay()            {return this->way;}
bool BPInternalNode::isOverFull()       {return signposts.size() > signCapacity;}
bool BPInternalNode::isLeaf()           {return false;}
int BPInternalNode::getDepth(int depth) {return children[0]->getDepth(depth+1);}
vector<BPNode*>* BPInternalNode::getChildren() {return &children;}



int BPInternalNode::numChildren() {
    return children.size();
}



void BPInternalNode::receiveChild(BPNode* givenChild, int givenPost)
{
    auto newFrontChild = children.begin();
    children.insert(newFrontChild, givenChild);

    auto newFrontPost = signposts.begin();
    signposts.insert(newFrontPost, givenPost);
}




void BPInternalNode::giveChild(BPInternalNode* receiver) {
        // pop child
        BPNode* popChild = children.back();
        children.pop_back();
        
        // pop signpost
        int popPost = *prev(signposts.end());
        
        
        // give child and signpost
        receiver->receiveChild(popChild, popPost);

}


BPNode* BPInternalNode::split() {
    // redistribute children to a new node
    BPInternalNode* sibling = new BPInternalNode(way, pageSize);
    while (sibling->numChildren() != this->children.size()+1 || sibling->numChildren() != this->children.size()) {
        giveChild(sibling);
    }
    return sibling;
}


int BPInternalNode::viewSign1() {
    auto front = signposts.begin();
    return *front;
}

int BPInternalNode::getSign1()
{
    auto front = signposts.begin();
    int result = *front;
    signposts.erase(front);
    return result;
}


void BPInternalNode::sortedInsert(BPNode* newChild) {
    // This is where keys are stolen from internal nodes created during splits
    // Insert signpost:
    int newKey = newChild->getSign1();

    // insert signpost
    auto currSign = signposts.begin();
    while (true) // BAD. find alternate approach?
    {
        if (currSign == signposts.end())
        {
            signposts.push_back(newKey);
            break;
        }
        else if (*currSign > newKey) // should never be equal - no dupes
        {
            signposts.insert(currSign, newKey);
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
        else if ((*currChild)->viewSign1() > newKey) // should never be equal - no dupes
        {
            children.insert(currChild, newChild); // NOTE: seems weird to dereference here. Am I making the right choice?
            break;
        }
        currChild++;
    }
}

BPNode* BPInternalNode::promote(BPNode* rep) {
    // add the new child node's relevant key to this internal's signposts
    // 1. add the newly created node to the children list
    sortedInsert(rep);


    // 2: splitting
    // a. if we're full, split the parent and redistribute children
    BPNode* splitResult = NULL;
    if (isOverFull())
    {
        splitResult = split();
        // a.5: IF THIS IS THE ROOT, create new internal node. make this node and its spouse children of that node. REMOVE AND GIVE spouse's first key to the new parent. return new parent.
        if (isRoot()) {
            BPInternalNode* newParent = new BPInternalNode(way, pageSize);
            newParent->isRoot();
            this->notRoot();
            // TODO TODO TODO: ADD CHILDREN TO PARENT HERE. REMEMBER TO STEAL A KEY
        }
    }
    else {
        return NULL;
    }

    return splitResult;
}





// when inserting on internal nodes that are children, add the result of insertion to the children list IF its pointer is different from the one you inserted on.
BPNode* BPInternalNode::insert(Item newItem) {
    BPNode* result{};
    for (int i = 0; i < signposts.size(); i++) 
    {
        if (i == signposts.size()-1)                                                            // BACK CHILD
        {
            if (newItem.getKey1() < signposts[i])
            {
                result = children[i]->insert(newItem);
                if (result == NULL) { // no split
                    return this;
                }
                else {
                    return promote(result);
                }
            }
            else {
                result = children[signposts.size()]->insert(newItem);
                if (result == NULL) { // no split
                    return this;
                }
                else {
                    return promote(result);
                }
            }
        }
        else if (i == 0 && newItem.getKey1() < signposts[i])                                    // FRONT CHILD
        {
            result = children[0]->insert(newItem);
            if (result == NULL) { // no split
                return this;
            }
            else {
                return promote(result);
            }
        }
        
        if (newItem.getKey1() >= signposts[i-1] && newItem.getKey1() < signposts[i]) {     // MIDDLE CHILD
            result = children[i]->insert(newItem);
            if (result == NULL) { // no split
                return this;
            }
            else {
                return promote(result);
            }
        }
    }
}


int BPInternalNode:: remove(int deleteIt) {
    return -99; // placeholder for compiler
}



vector<Item> BPInternalNode::search(int findIt) {

}


// "inorder traversal" that prints the root half-way through iterating through subtrees
void BPInternalNode::print(int depth) {
    for (int i = 0; i < children.size(); i++)
    {
        if (i == children.size() / 2) // Print this
        {
            // Print this:
            for (int i = 0; i < depth; i++)
            {
                cout << "     ";
            }
            cout << "I:";
            for (int i = 0; i < signposts.size(); i++)
            {
                cout << signposts[i];
                cout << ",";
            }
            cout << endl;
        }
        // Print current child
        children[i]->print(depth+1);
    }
}


