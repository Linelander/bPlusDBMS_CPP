#include <cstddef>
#include<iostream>
#include "Item.h"
#include "BPNode.h"
#include "BPInternalNode.h"
#include "BPLeaf.h"
#include <vector>

using namespace std;

#ifndef BP_TREE
#define BP_TREE

template <typename T>
class BPlusTree {
    public:
        BPlusTree(int way, int keyIndex);
        BPlusTree(int way, int keyIndex, size_t nonstandardSize);
        void insert(Item newItem);
        int remove(int deleteIt);
        vector<Item> search(int findIt);
        void print();
        int getDepth();

    private:
        int itemKeyIndex;
        BPNode<T>* root{};
        size_t pageSize = 4096;
};

#endif