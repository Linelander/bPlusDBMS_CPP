#include<iostream>
using namespace std;

class BPInternalNode {
    
    public:
        int getKey();
        int getWay();
        bool atLeastHalfFull();
        int insert(Item);
        int delete(Item);


    private:
        int key;
        int way{};
        list<Item> items; // list of items
        list<BPInternalNode> children; // list of BPInternalNodes
        BPInternalNode overflow = null;
}