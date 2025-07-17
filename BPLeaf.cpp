#include <cstddef>
#include<iostream>
#include "BPLeaf.h"
#include "Item.h"
#include <memory>
#include <vector>
#include <iterator>
#include "BPInternalNode.h"




using namespace std;

// FIELDS
// size_t pageSize{4096};
// int way{};
// vector<Item> items;
// // BPLeaf* overflow = NULL; // Doing this later.
// BPLeaf* neighbor{}; // Linked list of leaves

// METHODS
BPLeaf::BPLeaf(int way) {
    this->way = way;
}

BPLeaf::BPLeaf(int way, size_t nonstandardSize) {
    this->way = way;
    this->pageSize = nonstandardSize;
}

// Short Methods
bool BPLeaf::           isRoot() {return rootBool;}
void BPLeaf::           makeRoot() {rootBool = true;}
void BPLeaf::           notRoot() {rootBool = false;}
int BPLeaf::            getWay() {return this->way;}
bool BPLeaf::           isLeaf() {return true;}
BPLeaf* BPLeaf::        getNeighbor() {return neighbor;}
vector<Item>* BPLeaf::  accessItems() {return &items;}
int BPLeaf::            numItems() {return items.size();};
void BPLeaf::           setNeighbor(BPLeaf* newNeighbor) {neighbor = newNeighbor;}
size_t BPLeaf::size() {
    size_t leafSize = sizeof(BPLeaf) + (items.size() * sizeof(Item));
    // cout << leafSize;
    return leafSize;
} // Get the size of this leaf and its items


bool BPLeaf::checkOverflow() {
    size_t currSize = size();
    return (currSize > pageSize);
} // Is it time to split?


int BPLeaf::viewSign1() {
    return items.begin()->getKey1();
}

int BPLeaf::getSign1()
{
     auto front = items.begin();
     int result = front->getKey1();
     return result;
}


/*
    This implementation is a "rightward" split
 */
BPNode* BPLeaf::split()
{
    
    
    // Fill the new leaf half way
    int i = 0;
    BPLeaf *newLeaf = new BPLeaf(way, pageSize);
    while (newLeaf->numItems() != this->items.size() && newLeaf->numItems() != this->items.size()+1) // new leaf gets half of keys (rounds up for total odd number)
    {
        Item pop = items.back();
        items.pop_back();
        
        auto newFront = newLeaf->accessItems()->begin();
        *newLeaf->accessItems()->insert(newFront, pop);
        i++;
    }
    newLeaf->setNeighbor(this->neighbor);
    this->setNeighbor(newLeaf);
    
    
    
    // If this leaf node is the root, we need to return a new parent of both of these children
    if (isRoot())
    {
        BPInternalNode* newParent = new BPInternalNode(way, pageSize);
        newParent->makeRoot();
        
        this->notRoot();

        vector<BPLeaf*> adopt = {this, newLeaf};
        newParent->becomeFirstInternalRoot(adopt);

        return newParent;
    }

    return newLeaf; // the parent needs to add this to its list of children
}


/*
    IN PROGRESS
*/
BPNode* BPLeaf::insert(Item newItem) {
    if (items.size() == 0) {
        items.push_back(newItem);
        return NULL;
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



int BPLeaf::remove(int deleteIt) {
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



vector<BPNode*>* BPLeaf::getChildren() {
    return NULL;
}
