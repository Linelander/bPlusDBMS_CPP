#include<iostream>
#include "BPLeaf.h"
#include "Item.h"
#include <list>
using namespace std;

// FIELDS
int way{};
int capacity{};
list<Item> items;
BPLeaf* overflow = NULL;

// METHODS
BPLeaf::BPLeaf(int way) {
    this->way = way;
}

int BPLeaf::getWay() {return this->way;}
bool BPLeaf::atLeastHalfFull() {return (this->items.size() >= this->way/2);}
bool BPLeaf::isFull() {}
bool BPLeaf::isLeaf() {return true;}



int BPLeaf::insert(Item newItem) {

}

int BPLeaf::del(BPKey deleteIt) {

}

list<Item> BPLeaf::search(BPKey findIt)
{

};
