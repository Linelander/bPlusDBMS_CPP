#include<iostream>
#include "BPNode.h"
#include "Item.h"
#include "BPLeaf.h"

using namespace std;

#ifndef BP_INTERNAL_NODE
#define BP_INTERNAL_NODE

template<typename T>
class BPInternalNode : public BPNode<T> {
    
    public:
    bool isRoot();
    void makeRoot();
    void notRoot();
    BPInternalNode(int way, int keyIndex);
    BPInternalNode(int way, int keyIndex, size_t nonstandardSize);
    // int getKey();
    int getWay();
    int numChildren();
    void becomeFirstInternalRoot(vector<BPLeaf<T>*> newChildren);
    void becomeInternalRoot(vector<BPNode<T>*> newChildren);
    void receiveChild(BPNode<T>* givenChild, int givenPost);
    void giveChild(BPInternalNode* receiver);
    void sortedInsert(BPNode<T>* newChild);
    T viewSign1();
    T getSign1();
    BPNode<T>* split();
    BPNode<T>* promote(BPNode<T>* rep);
    BPNode<T>* insert(Item newItem);
    int remove(int deleteIt);
    vector<Item> search(int findIt);
    void setWay(int way);
    bool isOverFull();
    bool isLeaf();
    void print(int depth);
    int getDepth(int depth);
    vector<BPNode<T>*>* getChildren();
    
    
    private:
        int itemKeyIndex;
        bool rootBool{false};
        int pageSize = 4096;
        int way{};
        int signCapacity{};
        vector<T> signposts; // MUST LIMIT THE SIZE OF THIS LIST
        vector<BPNode<T>*> children{};
};

#endif