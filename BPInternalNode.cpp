#include <cstddef>
#include<iostream>
#include "BPInternalNode.h"
#include "Item.h"
#include <string>
using namespace std;

// FIELDS
size_t pageSize{4096};
int way{};          // max no. of children
int signCapacity{}; // max no. of signposts
vector<int> signposts{};
vector<BPNode> children{};

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
bool BPInternalNode::atLeastHalfFull()  {} // is this only for leaves?
int BPInternalNode::getWay()            {return this->way;}
bool BPInternalNode::isFull()           {return signposts.size() >= signCapacity;}
bool BPInternalNode::isLeaf()           {return false;}
int BPInternalNode::getDepth(int depth) {return children[0].getDepth(depth+1);}


void BPInternalNode::sortedInsert(int newKey) {

}


BPNode* BPInternalNode::split() {
    // redistribute children to a new leaf node
}


BPNode* BPInternalNode::promote(BPNode* rep) {
    // 1. add the newly created node to the children list
    if (rep->isLeaf()) {
        sortedInsert(rep.getKey1());
    }
    else {

    }
    // add the new child node's relevant key to this internal's signposts
    // PLACEHOLDER

    // 2: splitting
    // a. if we're full, split the parent and redistribute children
    BPNode* splitResult = NULL;
    if (isFull())
    {
        splitResult = split();
    }
    else {
        return NULL;
    }

    // a.5: IF THIS IS THE ROOT, create new internal node. make this node and its spouse children of that node. REMOVE AND GIVE spouse's first key to the new parent. return new parent.
    if (isRoot()) {
        BPInternalNode* newParent = new BPInternalNode(way, pageSize);
        // redistribute
    }
    
    // b: ELSE, REMOVE AND RETURN new spouse's first key... or the node or something?
}

// when inserting on internal nodes that are children, add the result of insertion to the children list IF its pointer is different from the one you inserted on.
BPNode* BPInternalNode::insert(Item newItem) {
    BPNode* result{};
    for (int i = 0; i < signposts.size(); i++) 
    {
        if (i == 0 && newItem.getKey1() < signposts[i])                                         // FRONT CHILD
        {
            result = children[0].insert(newItem);
            if (result == NULL) { // no split
                return this;
            }
            else {
                return promote(result); // PROMOTE MUST ALSO SPLIT
            }
        }
        else if (i == signposts.size()-1 && newItem.getKey1() >= signposts[i])                  // BACK CHILD
        {
            result = children[signposts.size()].insert(newItem);
            if (result == NULL) { // no split
                return this;
            }
            else {
                return promote(result); // PROMOTE MUST ALSO SPLIT
            }
        }
        else if (newItem.getKey1() >= signposts[i-1] && newItem.getKey1() < signposts[i]) {     // MIDDLE CHILD
            result = children[i].insert(newItem);
            if (result == NULL) { // no split
                return this;
            }
            else {
                return promote(result);
            }
        }
    }
}


int BPInternalNode:: del(Item) {

}


vector<Item> BPInternalNode::search(int findIt) {

}


// "inorder traversal" that prints the root half-way through iterating through subtrees
void print(int depth) {
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
        children[i].print(depth+1);
    }
}


