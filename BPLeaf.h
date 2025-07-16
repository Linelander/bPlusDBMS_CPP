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
        bool isRoot();
        void makeRoot();
        void notRoot();    
        BPLeaf(int way);
        BPLeaf(int way, size_t nonstandardSize);
        void setPageSize(size_t nonstandardSize);
        int getWay();
        int getDepth(int depth);
        BPNode* insert(Item newItem);
        int remove(int deleteIt);
        vector<Item> search(int findIt);
        int getSign1();
        // void setWay(int way);
        bool isLeaf();
        void print(int depth);
        void setNeighbor(BPLeaf*);
        BPLeaf* getNeighbor();
        size_t size();
        bool checkOverflow();
        BPNode* split();
        void promote();
        vector<Item>* accessItems();
        vector<BPNode> getChildren();

        
    private:
        bool rootBool{false};
        size_t pageSize;
        int way{};
        bool checkHasRoom();
        int remainingSpace{};
        vector<Item> items;
        // BPLeaf* overflow = NULL;
        BPLeaf* neighbor = NULL;
};

#endif