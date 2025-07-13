#include<iostream>
#include "Item.h"
#include <vector>
#include "BPKey.h"
#include "BPNode.h"
using namespace std;

#ifndef BP_LEAF
#define BP_LEAF

class BPLeaf : public BPNode {
    public:
        BPLeaf(int way);
        int getWay();
        int getDepth(int depth);
        bool atLeastHalfFull();
        int insert(Item newItem);
        int del(BPKey deleteIt);
        vector<Item> search(BPKey findIt);
        void setWay(int way);
        bool isFull();
        bool isLeaf();
        void print(int depth);
        vector<BPNode> getChildren();

        
    private:
        int way{};
        int capacity{};
        int remainingSpace{};
        vector<Item> items;
        BPLeaf* overflow = NULL;
        BPLeaf* neighbor = NULL;
};

#endif