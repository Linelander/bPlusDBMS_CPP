#include <cstddef>
#include<iostream>
#include "BPLeaf.h"
#include "Item.h"
#include <vector>
#include <iterator>




using namespace std;

// FIELDS
size_t pageSize{4096};
int way{};
int capacity{};
vector<Item> items;
// BPLeaf* overflow = NULL;
BPLeaf* neighbor{}; // Linked list of leaves

// METHODS
BPLeaf::BPLeaf(int way) {
    this->way = way;
}

BPLeaf::BPLeaf(int way, size_t nonstandardSize) {
    this->way = way;
    this->pageSize = nonstandardSize;
}

// Short Methods
int BPLeaf::            getWay() {return this->way;}
bool BPLeaf::           isLeaf() {return true;}
BPLeaf* BPLeaf::        getNeighbor() {return neighbor;}
vector<Item>* BPLeaf::  accessItems() {return &items;}
size_t BPLeaf::         size() {return sizeof(BPLeaf) + (items.size() * sizeof(Item));} // Get the size of this leaf and its items
bool BPLeaf::           checkOverflow() {return (size() > pageSize);} // Is it time to split?
void BPLeaf::           setNeighbor(BPLeaf* newNeighbor) {neighbor = newNeighbor;}

/*
    This implementation is a "rightward" split
 */
void BPLeaf::split()
{
    // Fill the new leaf half way
    BPLeaf newLeaf = BPLeaf(way);
    while (newLeaf.size() < pageSize / 2)
    {
        Item pop = *prev(items.end());
        auto newFront = newLeaf.accessItems()->begin();
        *newLeaf.accessItems()->insert(newFront, pop);
    }
    newLeaf.setNeighbor(this->neighbor);
    this->setNeighbor(&newLeaf);
}



/*
    IN PROGRESS
*/
int BPLeaf::insert(Item newItem) {
    if (items.size() == 0) {
        items.push_back(newItem);
        return 0;
    }

    auto curr = items.begin();
    while (curr != items.end())
    {
        if (curr->getKey1() >= newItem.getKey1() || curr == items.end()) // TODO: write comparator for Items
        {
            items.insert(curr, newItem);
            break;
        }
        curr++;
    }

    if (checkOverflow())
    {
        split();
    }
    return 0;
}



void BPLeaf::promote() {
    // does this happen here or in the parent?
}



int BPLeaf::del(int deleteIt) {

}



vector<Item> BPLeaf::search(int findIt) {

}



void BPLeaf::print(int depth) {
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



int BPLeaf::getDepth(int depth) {
    return depth;
}



vector<BPNode> BPLeaf::getChildren() {
    return vector<BPNode>{};
}
