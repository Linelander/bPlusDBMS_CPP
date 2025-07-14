#include<iostream>
#include "Item.h"
#include "BPNode.h"
#include "BPInternalNode.h"
#include "BPLeaf.h"
#include <vector>

using namespace std;

#ifndef BP_TREE
#define BP_TREE

class BPlusTree {
    public:
        BPlusTree(int way);
        void insert(Item newItem);
        int del(int deleteIt);
        vector<Item> search(int findIt);
        void print();
        int getDepth();

    private:
        BPNode* root;
};

#endif