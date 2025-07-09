#include<iostream>
#include "BPNode.h"
#include"Item.h"
#include "BPKey.h"
using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

class BPInternalNode : public BPNode {
    
    public:
        BPInternalNode(int way);
        BPKey getKey();
        int getWay();
        bool atLeastHalfFull();
        int insert(Item newItem);
        int del(Item deleteIt);
        list<Item> search(BPKey findIt);
        void setWay(int way);
        bool isFull();
        bool isLeaf();


    private:
        int way{};
        int capacity{};
        list<BPKey> signposts; // MUST LIMIT THE SIZE OF THIS LIST
        std::list<BPNode> children;
};

#endif