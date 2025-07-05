#include<iostream>
#include "BpInternalNode.h"
#include "Item.h"
using namespace std;

class BPInternalNode {
    
    public:
        int getKey() {
            return this.key; // What type should keys be?
        }
        int getWay() {
            return this.way;
        }
        bool atLeastHalfFull()
        {
            return (this.items.size() >= this.way/2);
        }

        
        int insert(Item) {

        }
        int delete(Item);


    private:
        int key;
        int way{};
        list<Item> items; // list of items
        list<BPInternalNode> children; // list of BPInternalNodes
        BPInternalNode overflow = null;
}