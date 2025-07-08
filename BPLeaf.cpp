#include<iostream>
#include "BPLeaf.h"
#include "Item.h"
#include <list>
using namespace std;

// FIELDS
int way{};
list<Item> items;
BPLeaf* overflow = NULL;

// METHODS
BPLeaf::BPLeaf(int way) {
    this->way = way;
}

int BPLeaf::getWay() {return this->way;}
bool BPLeaf::atLeastHalfFull() {return (this->items.size() >= this->way/2);}


int BPLeaf::insert(Item newItem) {
    return -1;
}

int BPLeaf::del(BPKey deleteIt) {
    return -1;
}

list<Item> BPLeaf::search(BPKey findIt)
{
    return {};
};
