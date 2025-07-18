#include<iostream>
#include "Item.h"
#include "BPNode.h"
#include "BPInternalNode.h"
#include "BPLeaf.h"

using namespace std;

#ifndef BP_TREE
#define BP_TREE

class BPlusTree {
    public:
        BPlusTree(int way);
        int insert(Item newItem);
        int del(BPKey deleteIt);
        list<Item> search(BPKey findIt);

    private:
        BPNode* root;
};

#endif