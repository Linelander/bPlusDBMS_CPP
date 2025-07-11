#include<iostream>
#include "BPInternalNode.h"
#include "Item.h"
#include <string>
using namespace std;

// FIELDS
int way{};
int capacity{};
vector<BPKey> signposts{};
vector<BPNode> children{};

// CONSTRUCTORS
BPInternalNode::BPInternalNode(int way) {

}
    
// METHODS
int BPInternalNode::getWay() {return this->way;}
bool BPInternalNode::atLeastHalfFull() {}
bool BPInternalNode::isFull() {}
bool BPInternalNode::isLeaf() {return false;}
int BPInternalNode::getDepth(int depth)
{
    return children[0].getDepth(depth+1);
}


int BPInternalNode::insert(Item) {

}



int BPInternalNode:: del(Item) {

}

vector<Item> BPInternalNode::search(BPKey findIt)
{

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
            cout << "#kids: ";
            cout << signposts.size();
            for (int i = 0; i < signposts.size(); i++)
            {
                cout << signposts[i].getFullKey();
            }
        }
        // Print current child
        children[i].print(depth+1);
    }
}


