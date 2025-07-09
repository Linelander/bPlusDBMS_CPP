#include<iostream>
#include "Item.h"
#include <list>
#include "BPKey.h"
#include "BPNode.h"
using namespace std;

#ifndef BP_LEAF
#define BP_LEAF

class BPLeaf : public BPNode {
    public:
        BPLeaf(int way);
        int getWay();
        bool atLeastHalfFull();
        int insert(Item newItem);
        int del(BPKey deleteIt);
        list<Item> search(BPKey findIt);
        void setWay(int way);
        bool isFull();
        bool isLeaf();

        
    private:
        int way{};
        int capacity{};
        list<Item> items;
        BPLeaf* overflow = NULL;
        BPLeaf* neighbor = NULL;
};

#endif