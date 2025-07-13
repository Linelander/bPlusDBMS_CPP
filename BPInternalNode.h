#include<iostream>
#include "BPNode.h"
#include"Item.h"
using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

class BPInternalNode : public BPNode {
    
    public:
        BPInternalNode(int way);
        int getKey();
        int getWay();
        bool atLeastHalfFull();
        int insert(Item newItem);
        int del(Item deleteIt);
        vector<Item> search(int findIt);
        void setWay(int way);
        bool isFull();
        bool isLeaf();
        void print(int depth);
        int getDepth(int depth);
        vector<BPNode> getChildren();


    private:
        int way{};
        int capacity{};
        vector<int> signposts; // MUST LIMIT THE SIZE OF THIS LIST
        vector<BPNode> children;
};

#endif