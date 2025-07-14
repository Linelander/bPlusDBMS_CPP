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
void BPLeaf::           setNeighbor(BPLeaf* newNeighbor) {neighbor = newNeighbor;}


bool BPLeaf::           checkOverflow() {
    size_t currSize = size();
    return (currSize > pageSize);
} // Is it time to split?

/*
    This implementation is a "rightward" split
 */
BPNode* BPLeaf::split()
{
    // If this leaf node is the root, we need to return a new parent of both of these children
    
    
    // Fill the new leaf half way
    BPLeaf *newLeaf = new BPLeaf(way, pageSize);
    while (newLeaf->accessItems()->size() != this->items.size()+1 || newLeaf->accessItems()->size() != this->items.size()) // new leaf gets half of keys (rounds up for total odd number)
    {
        Item pop = *prev(items.end());
        items.pop_back();

        auto newFront = newLeaf->accessItems()->begin();
        *newLeaf->accessItems()->insert(newFront, pop);
    }
    newLeaf->setNeighbor(this->neighbor);
    this->setNeighbor(newLeaf);

    return newLeaf; // the parent needs to add this to its list of children
}


/*
    IN PROGRESS
*/
BPNode* BPLeaf::insert(Item newItem) {
    if (items.size() == 0) {
        items.push_back(newItem);
        return this;
    }

    auto curr = items.begin();
    while (true) // BAD. find alternate approach?
    {
        if (curr->getKey1() >= newItem.getKey1()) // TODO: write comparator for Items
        {
            items.insert(curr, newItem);
            break;
        }
        else if (curr == items.end())
        {
            items.push_back(newItem);
            break;
        }
        curr++;
    }

    if (checkOverflow())
    {
        return split();
    }
    return NULL;
}



void BPLeaf::promote() {
    // does this happen here or in the parent?
}



int BPLeaf::del(int deleteIt) {
    return -99;
}



vector<Item> BPLeaf::search(int findIt) {
    return vector<Item>{}; // PLACEHOLDER FOR COMPILER
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
    cout << "L:";
    cout << items[0].getKey1();
    cout << endl;
}



int BPLeaf::getDepth(int depth) {
    return depth;
}



vector<BPNode> BPLeaf::getChildren() {
    return vector<BPNode>{};
}
