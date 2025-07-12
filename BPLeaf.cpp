#include<iostream>
#include "BPLeaf.h"
#include "Item.h"
#include <vector>
using namespace std;

// FIELDS
int way{};
int capacity{};
vector<Item> items;
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

vector<Item> BPLeaf::search(BPKey findIt)
{

}

void print(int depth) {
    // Print this:
    if (items.size() == 0)
    {
        cout << "empty";
        return;
    }


    for (int i = 0; i < depth; i++)
    {
        cout << "     ";
    }
    cout << "#keys: ";
    cout << items.size();
    for (int i = 0; i < items.size(); i++)
    {
        cout << items[i].getFullKey();
    }
}

int getDepth(int depth) {
    return depth;
}

vector<BPNode> getChildren() {
    return vector<BPNode>{};
}
