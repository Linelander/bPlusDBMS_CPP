#include<iostream>
#include "BPNode.h"
#include"Item.h"
using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

class BPInternalNode : public BPNode {
    
    public:
        BPInternalNode(int way);
        BPInternalNode(int way, size_t nonstandardSize);
        int getKey();
        int getWay();
        bool atLeastHalfFull();
        void sortedInsert(int newKey);
        BPNode* split();
        BPNode* promote(BPNode* rep);
        BPNode* insert(Item newItem);
        int del(Item deleteIt);
        vector<Item> search(int findIt);
        void setWay(int way);
        bool isFull();
        bool isLeaf();
        void print(int depth);
        int getDepth(int depth);
        vector<BPNode> getChildren();


    private:
        int pageSize{4096};
        int way{};
        int signCapacity{};
        vector<int> signposts; // MUST LIMIT THE SIZE OF THIS LIST
        vector<BPNode> children;
};

#endif