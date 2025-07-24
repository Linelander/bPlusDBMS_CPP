#include <cstddef>
#include<iostream>
#include "BPLeaf.h"
#include "Item.h"
#include <memory>
#include <vector>
#include <iterator>
#include "BPInternalNode.h"
#include <unistd.h>


using namespace std;


// METHODS
BPLeaf::BPLeaf(int way) {
    this->way = way;
    long foundSize = sysconf(_SC_PAGESIZE);
    pageSize = foundSize;
    cout << "test";
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
        if (curr->getKey1() == newItem.getKey1()) // Duplicate keys not yet supported
        {
            cout << "ERROR: INSERTION - Duplicate Keys not yet supported." << endl;
            return NULL;
        }
        
        if (curr->getKey1() > newItem.getKey1()) // TODO: write swappable comparators for Items searching on multiple keys
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
        cout << "EL";
        return;
    }


    for (int i = 0; i < depth; i++)
    {
        cout << "                    ";
    }
    cout << "D" << depth << "-L:";
    cout << items[0].getKey1();
    cout << endl;
}



int BPLeaf::getDepth(int depth) {
    return depth;
}



vector<BPNode*>* BPLeaf::getChildren() {
    return NULL;
}
