#include<iostream>
#include "BPInternalNode.h"
#include "Item.h"
using namespace std;


    

int BPInternalNode::getKey() {
    return this->key; // What type should keys be?
}
int BPInternalNode::getWay() {
    return this->way;
}


int BPInternalNode::insert(Item) {

}



int BPInternalNode:: del(Item) {

}


int key;
int way{};

list<generic?> children; // list of BPInternalNodes

BPInternalNode overflow = {};


