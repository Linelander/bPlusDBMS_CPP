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

int BPLeaf::getWay() {return this->way;}
bool BPLeaf::atLeastHalfFull() {return (this->items.size() >= this->way/2);}
bool BPLeaf::isFull() {}
bool BPLeaf::isLeaf() {return true;}



void BPLeaf::setPageSize(size_t nonstandardSize) {
    pageSize = nonstandardSize;
}


/*
    Get the size of this leaf and its items
*/
size_t BPLeaf::size() {
    return sizeof(BPLeaf) + (items.size() * sizeof(Item));
}


/*
    Is it time to split?
*/
bool BPLeaf::checkOverflow() {
    return (size() > pageSize);
}

vector<Item>* BPLeaf::accessItems() {
    return &items;
}

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
