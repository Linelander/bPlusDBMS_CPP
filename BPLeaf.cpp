#include<iostream>
#include "BPLeaf.h"
#include "Item.h"
#include <vector>
#include <iterator>

extern const int PAGE_SIZE = 4096;

using namespace std;

// FIELDS
int way{};
int capacity{};
vector<Item> items;
BPLeaf* overflow = NULL;
BPLeaf* neighbor{};

// METHODS
BPLeaf::BPLeaf(int way) {
    this->way = way;
}

int BPLeaf::getWay() {return this->way;}
bool BPLeaf::atLeastHalfFull() {return (this->items.size() >= this->way/2);}
bool BPLeaf::isFull() {}
bool BPLeaf::isLeaf() {return true;}


bool checkHasRoom()
{
    size_t currentSize = sizeof(BPLeaf) + (items.size() * sizeof(Item));
    return (PAGE_SIZE - currentSize >= sizeof(Item));
}






/*
IN PROGRESS
*/
int BPLeaf::insert(Item newItem) {
    if (!checkHasRoom())
    {
        *overflow = BPLeaf(way);
        overflow->insert(newItem);
    }
    
    if (items.size() == 0) {
        items.push_back(newItem);
    }
    else {
        auto curr = items.begin();
        while (curr != items.end())
        {
            // if item at iterator is bigger than newItem or we reach the end, insert
            if (curr->getKey1() >= newItem.getKey1() || curr == items.end()) // TODO: write comparator for Items
            {
                items.insert(curr, newItem);
                break;
            }
            curr++;
        }
    }

    // now split?
}


void split()
{

}

void promote()
{
    // does this happen here or in the parent?
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
