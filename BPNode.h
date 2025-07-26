#include <vector>
#include "Item.h"

using namespace std;


#ifndef BPNODE_H
#define BPNODE_H

template <typename T>
class BPNode {
    public:
        virtual T viewSign1() = 0;
        virtual T getSign1() = 0;
        virtual void makeRoot() = 0;
        virtual void notRoot() = 0;
        virtual bool isRoot() = 0;
        virtual int getWay() = 0;
        virtual int getDepth(int depth) = 0;
        virtual BPNode* insert(ItemInterface* newItem) = 0;
        virtual int remove(int deleteIt) = 0;
        virtual vector<ItemInterface*> search(T findIt) = 0;
        virtual ItemInterface* singleSearch(T findIt) = 0;
        virtual bool isLeaf() = 0;
        virtual void print(int depth) = 0;
        virtual vector<BPNode*>* getChildren() = 0;
};

#endif