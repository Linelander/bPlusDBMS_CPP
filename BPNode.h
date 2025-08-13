#include <vector>
#include "Item.h"

using namespace std;


#ifndef BPNODE_H
#define BPNODE_H

enum class RemovalAction {
    DEFAULT,
    SIMPLE_REMOVAL,
    STOLE_FROM_LEFT,
    STOLE_FROM_RIGHT,
    MERGED_INTO_LEFT,
    MERGED_INTO_RIGHT
};

enum class LastLocation {
    LEAF,
    INTERNAL
};

template <typename T>
struct RemovalResult {
    T rightSubtreeMin = nullptr;
    ItemInterface* removedItem;    // Removed item
    RemovalAction action;          // Events of removal
    LastLocation lastLocation = LastLocation::LEAF;
    
    RemovalResult(ItemInterface* item, RemovalAction act) 
        : removedItem(item), action(act) {}
};

template <typename T>
class BPNode {
    private:
        void printKey(int key);
        void printKey(const AttributeType& attr);
    
    public:
        virtual ~BPNode() = default;
        virtual T viewSign1() = 0;
        virtual T getSign1() = 0;
        virtual void makeRoot() = 0;
        virtual void notRoot() = 0;
        virtual bool isRoot() = 0;
        virtual int getDepth(int depth) = 0;
        virtual BPNode<T>* insert(ItemInterface* newItem) = 0;
        virtual ItemInterface* remove(T deleteIt) = 0;
        virtual ItemInterface* singleKeySearch(T findIt) = 0;
        virtual bool isLeaf() = 0;
        virtual void print(int depth) = 0;
};

#endif