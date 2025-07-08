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
        BPKey getKey();
        int getWay();
        bool atLeastHalfFull();
        int insert(Item);
        int del(Item);
        int search(BPKey);


    private:
        int way{};
        list<Item> items;
        BPLeaf* overflow = NULL;
        BPLeaf* neighbor = NULL;
};

#endif