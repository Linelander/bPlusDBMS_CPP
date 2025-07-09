#include <list>
#include "Item.h"
#include "BPKey.h"
using namespace std;

#ifndef BPNODE_H
#define BPNODE_H

class BPNode {
    public:
        virtual int getWay() = 0;
        virtual int insert(Item newItem) = 0;
        virtual int del(BPKey deleteIt) = 0;
        virtual list<Item> search(BPKey findIt) = 0;
        virtual void setWay(int way) = 0;
        virtual bool isFull() = 0;
        virtual bool isLeaf() = 0;
        virtual bool isHalfFull() = 0;
};

#endif