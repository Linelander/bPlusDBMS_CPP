#include "BPNode.h"
#include<iostream>
#include"Item.h"
using namespace std;

class BPInternalNode {
    
    public:
        int getKey();
        int getWay();
        bool atLeastHalfFull();
        int insert(Item);
        int del(Item);


    private:
        int key;
        int way{};
        std::list<generic> children; // list of BPInternalNodes
        BPInternalNode overflow = null;
}