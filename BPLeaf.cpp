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
template <typename T>
BPLeaf<T>::BPLeaf(int way, int keyIndex) {
    this->way = way;
    long foundSize = sysconf(_SC_PAGESIZE);
    pageSize = foundSize;
    this->itemKeyIndex = keyIndex;
}

template <typename T>
BPLeaf<T>::BPLeaf(int way, int keyIndex, size_t nonstandardSize) {
    this->way = way;
    this->pageSize = nonstandardSize;
    this->itemKeyIndex = keyIndex;
}

// Short Methods
template <typename T>
bool BPLeaf<T>::           isRoot() {return rootBool;}
template <typename T>
void BPLeaf<T>::           makeRoot() {rootBool = true;}
template <typename T>
void BPLeaf<T>::           notRoot() {rootBool = false;}
template <typename T>
int BPLeaf<T>::            getWay() {return this->way;}
template <typename T>
bool BPLeaf<T>::           isLeaf() {return true;}
template <typename T>
BPLeaf<T>* BPLeaf<T>::        getNeighbor() {return neighbor;}
template <typename T>
vector<ItemInterface*>* BPLeaf<T>::  accessItems() {return &items;}
template <typename T>
int BPLeaf<T>::            numItems() {return items.size();};
template <typename T>
void BPLeaf<T>::           setNeighbor(BPLeaf<T>* newNeighbor) {neighbor = newNeighbor;}
template <typename T>
size_t BPLeaf<T>::size() {
    size_t leafSize = sizeof(BPLeaf) + (items.size() * sizeof(Item));
    // cout << leafSize;
    return leafSize;
} // Get the size of this leaf and its items


template <typename T>
bool BPLeaf<T>::checkOverflow() {
    size_t currSize = size();
    return (currSize > pageSize);
} // Is it time to split?


template <typename T>
int BPLeaf<T>::viewSign1() {
    return (*items.begin())->getPrimaryKey();
}

template <typename T>
int BPLeaf<T>::getSign1()
{
     auto front = items.begin();
     int result = (*front)->getPrimaryKey();
     return result;
}


/*
    This implementation is a "rightward" split
 */
template <typename T>
BPNode<T>* BPLeaf<T>::split()
{
    
    // TODO: get rid of accessItems()
    // Fill the new leaf half way
    int i = 0;
    BPLeaf *newLeaf = new BPLeaf(way, itemKeyIndex, pageSize);
    while (newLeaf->numItems() != this->items.size() && newLeaf->numItems() != this->items.size()+1) // new leaf gets half of keys (rounds up for total odd number)
    {
        ItemInterface* pop = items.back();
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
        BPInternalNode<T>* newParent = new BPInternalNode<T>(way, itemKeyIndex, pageSize);
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
template <typename T>
BPNode<T>* BPLeaf<T>::insert(ItemInterface* newItem) {
    if (items.size() == 0) {
        items.push_back(newItem);
        return NULL;
    }

    auto curr = items.begin();
    while (true) // BAD. find alternate approach?
    {
        if ((*curr)->getPrimaryKey() == newItem->getPrimaryKey()) // Duplicate keys not yet supported
        {
            cout << "ERROR: INSERTION - Duplicate Keys not yet supported." << endl;
            return NULL;
        }
        
        if ((*curr)->getPrimaryKey() > newItem->getPrimaryKey()) // TODO: write swappable comparators for Items searching on multiple keys
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



template <typename T>
int BPLeaf<T>::remove(int deleteIt) {
    return -99;
}



template <typename T>
vector<Item> BPLeaf<T>::search(int findIt) {
    return vector<Item>{}; // PLACEHOLDER FOR COMPILER
}



template <typename T>
void BPLeaf<T>::print(int depth) {
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
    cout << items[0]->getPrimaryKey();
    cout << endl;
}



template <typename T>
int BPLeaf<T>::getDepth(int depth) {
    return depth;
}



template <typename T>
vector<BPNode<T>*>* BPLeaf<T>::getChildren() {
    return NULL;
}
