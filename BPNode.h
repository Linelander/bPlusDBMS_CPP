#include "Item.h"
#include "BPKey.h"
#include <vector>
using namespace std;

#ifndef BPNODE_H
#define BPNODE_H

class BPNode {
    public:
        virtual int getWay() = 0;
        virtual int getDepth(int depth) = 0;
        virtual int insert(Item newItem) = 0;
        virtual int del(BPKey deleteIt) = 0;
        virtual vector<Item> search(BPKey findIt) = 0;
        virtual void setWay(int way) = 0;
        virtual bool isFull() = 0;
        virtual bool isLeaf() = 0;
        virtual bool atLeastHalfFull() = 0;
        virtual void print(int depth) = 0;
        virtual vector<BPNode> getChildren() = 0;
};

#endif