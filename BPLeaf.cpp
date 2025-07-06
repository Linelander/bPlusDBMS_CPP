#include<iostream>
#include "BPLeaf.h"
#include "Item.h"
#include <list>
using namespace std;



int BPLeaf::getKey() {return this.key;} // What type should keys be?
int BPLeaf::getWay() {return this.way;}
bool BPLeaf::atLeastHalfFull() {return (this.items.size() >= this.way/2)};


int BPLeaf::insert(Item) {

}
int BPLeaf::del(Item) {

}



int BPLeaf::key;
int BPLeaf::way{};

list<Item> items;
BPLeaf overflow = NULL;

