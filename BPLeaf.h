#include <cstddef>
#include<iostream>
#include "Item.h"
#include <vector>
#include "BPNode.h"
using namespace std;

#ifndef BP_LEAF
#define BP_LEAF

class BPLeaf : public BPNode {
    public:
        BPLeaf(int way);
        BPLeaf(int way, size_t nonstandardSize);
        void setPageSize(size_t nonstandardSize);
        int getWay();
        int getDepth(int depth);
        int insert(Item newItem);
        int del(int deleteIt);
        vector<Item> search(int findIt);
        // void setWay(int way);
        bool isLeaf();
        void print(int depth);
        void setNeighbor(BPLeaf*);
        BPLeaf* getNeighbor();
        size_t size();
        bool checkOverflow();
        void split();
        void promote();
        vector<Item>* accessItems();
        vector<BPNode> getChildren();

        
    private:
        size_t pageSize;
        int way{};
        int capacity{};
        bool checkHasRoom();
        int remainingSpace{};
        vector<Item> items;
        // BPLeaf* overflow = NULL;
        BPLeaf* neighbor = NULL;
};

#endif