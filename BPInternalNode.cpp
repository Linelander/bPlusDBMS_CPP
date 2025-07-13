#include<iostream>
#include "BPInternalNode.h"
#include "Item.h"
#include <string>
using namespace std;

// FIELDS
int way{};
int capacity{}; // max no. of signposts
vector<int> signposts{};
vector<BPNode> children{};

// CONSTRUCTORS
BPInternalNode::BPInternalNode(int way) {
    this->way = way;
    this->capacity = way+1;
}
    
// METHODS
bool BPInternalNode::atLeastHalfFull()  {} // is this only for leaves?
int BPInternalNode::getWay()            {return this->way;}
bool BPInternalNode::isFull()           {return signposts.size() >= capacity;}
bool BPInternalNode::isLeaf()           {return false;}
int BPInternalNode::getDepth(int depth) {return children[0].getDepth(depth+1);}


int BPInternalNode::insert(Item) {

}


int BPInternalNode:: del(Item) {

}


vector<Item> BPInternalNode::search(int findIt) {

}


// "inorder traversal" that prints the root half-way through iterating through subtrees
void print(int depth) {
    for (int i = 0; i < children.size(); i++)
    {
        if (i == children.size() / 2)
        {
            // Print this:
            for (int i = 0; i < depth; i++)
            {
                cout << "     ";
            }
            cout << "(#";
            cout << signposts.size();
            cout << ") ";
            for (int i = 0; i < signposts.size(); i++)
            {
                cout << signposts[i];
                cout << ",";
            }
        }
        // Print current child
        children[i].print(depth+1);
    }
}


