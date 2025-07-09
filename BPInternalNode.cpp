#include<iostream>
#include "BPInternalNode.h"
#include "Item.h"
using namespace std;

// FIELDS
int way{};
int capacity{};
list<BPKey> signposts{};
list<BPNode> children{};

// CONSTRUCTORS
BPInternalNode::BPInternalNode(int way) {

}
    
// METHODS
int BPInternalNode::getWay() {return this->way;}
bool BPInternalNode::atLeastHalfFull() {}
bool BPInternalNode::isFull() {}
bool BPInternalNode::isLeaf() {return false;}


int BPInternalNode::insert(Item) {

}



int BPInternalNode:: del(Item) {

}

list<Item> BPInternalNode::search(BPKey findIt)
{

}


