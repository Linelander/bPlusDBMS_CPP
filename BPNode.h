#include "Item.h"
#include <vector>
using namespace std;

#ifndef BPNODE_H
#define BPNODE_H

class BPNode {
    public:
        virtual int viewSign1() = 0;
        virtual int getSign1() = 0;
        virtual void makeRoot() = 0;
        virtual void notRoot() = 0;
        virtual bool isRoot() = 0;
        virtual int getWay() = 0;
        virtual int getDepth(int depth) = 0;
        virtual BPNode* insert(Item newItem) = 0;
        virtual int del(int deleteIt) = 0;
        virtual vector<Item> search(int findIt) = 0;
        virtual bool isLeaf() = 0;
        virtual void print(int depth) = 0;
        virtual vector<BPNode> getChildren() = 0;
};

#endif