#include<iostream>
#include "BPNode.h"
#include "Item.h"
#include "BPLeaf.h"

using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

class BPInternalNode : public BPNode {
    
    public:
    bool isRoot();
    void makeRoot();
    void notRoot();
    BPInternalNode(int way);
    BPInternalNode(int way, size_t nonstandardSize);
    int getKey();
    int getWay();
    int numChildren();
    void becomeFirstInternalRoot(vector<BPLeaf*> newChildren);
    void becomeInternalRoot(vector<BPNode*> newChildren);
    void receiveChild(BPNode* givenChild, int givenPost);
    void giveChild(BPInternalNode* receiver);
    void sortedInsert(BPNode* newChild);
    int viewSign1();
    int getSign1();
    BPNode* split();
    BPNode* promote(BPNode* rep);
    BPNode* insert(Item newItem);
    int remove(int deleteIt);
    vector<Item> search(int findIt);
    void setWay(int way);
    bool isOverFull();
    bool isLeaf();
    void print(int depth);
    int getDepth(int depth);
    vector<BPNode*>* getChildren();
    
    
    private:
        bool rootBool{false};
        int pageSize{4096};
        int way{};
        int signCapacity{};
        vector<int> signposts; // MUST LIMIT THE SIZE OF THIS LIST
        vector<BPNode*> children{};
};

#endif