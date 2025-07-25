#include <cstddef>
#include<iostream>
#include "Item.h"
#include <vector>
#include "BPNode.h"
using namespace std;

#ifndef BP_LEAF
#define BP_LEAF

template<typename T>
class BPLeaf : public BPNode<T> {
    public:
        bool isRoot();
        void makeRoot();
        void notRoot();    
        BPLeaf<T>(int way, int keyIndex);
        BPLeaf<T>(int way, int keyIndex, size_t nonstandardSize);
        void setPageSize(size_t nonstandardSize);
        int getWay();
        int getDepth(int depth);
        BPNode<T>* insert(ItemInterface* newItem);
        int remove(int deleteIt);
        vector<Item> search(int findIt);
        int viewSign1();
        int getSign1();
        // void setWay(int way);
        bool isLeaf();
        void print(int depth);
        void setNeighbor(BPLeaf<T>*);
        BPLeaf<T>* getNeighbor();
        int numItems();
        size_t size();
        bool checkOverflow();
        BPNode<T>* split();
        vector<ItemInterface*>* accessItems();
        vector<BPNode<T>*>* getChildren();

        
    private:
        int itemKeyIndex;
        bool rootBool{false};
        size_t pageSize = 4096;
        int way{};
        vector<ItemInterface*> items; // ItemInterface* or ItemInterface?
        // BPLeaf<T>* overflow = NULL;
        BPLeaf<T>* neighbor = NULL;
};

#endif