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





BPNode* BPInternalNode::split() {
    
}


BPNode* BPInternalNode::promote() {
    // stub
}


BPNode* BPInternalNode::insert(Item newItem) {
    BPNode* result{};
    for (int i = 0; i < signposts.size(); i++) 
    {
        if (i == 0 && newItem.getKey1() < signposts[i])  // FRONT CHILD
        {
            result = children[0].insert(newItem);
            if (result == NULL) {
                return this;
            }
            else {
                return promote(result); // PROMOTE MUST ALSO SPLIT
            }
        }
        else if (i == signposts.size()-1 && newItem.getKey1() >= signposts[i]) // BACK CHILD
        {
            result = children[signposts.size()].insert(newItem);
            if (result == NULL) {
                return this;
            }
            else {
                return promote(result); // PROMOTE MUST ALSO SPLIT
            }
        }
        else if (newItem.getKey1() >= signposts[i-1] && newItem.getKey1() < signposts[i]) {
            result = children[i].insert(newItem);
            if (result == NULL) {
                return 0;
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
            cout << "I:"
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


