#include<iostream>
#include "BPNode.h"
#include"Item.h"
#include "BPKey.h"
using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

class BPInternalNode : public BPNode {
    
    public:
        BPKey getKey();
        int getWay();
        bool atLeastHalfFull();
        int insert(Item);
        int del(Item);
        void setWay(int);


    private:
        list<BPKey> signposts; // MUST LIMIT THE SIZE OF THIS LIST
        int way{};
        std::list<BPNode> children; // list of Nodes 
};

#endif