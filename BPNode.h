#include <cstddef>
#include <vector>
#include <optional>

#include "Item.h"
#include "ItemInterface.h"
#include "Freelist.h"

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
    ItemInterface* removedItem;    // Removed item
    RemovalAction action;          // Events of removal
    LastLocation lastLocation = LastLocation::LEAF;
    
    RemovalResult(ItemInterface* item, RemovalAction act) 
        : removedItem(item), action(act) {}
};

template <typename T, int way>
class BPNode {
    private:
        void printKey(int key);
        void printKey(const AttributeType& attr);
        virtual BPNode<T, way>* getChild(int index) = 0;
    
    public:
        virtual void dehydrate() = 0;
        virtual void setNext(BPNode<T, way>* newNext) = 0;
        virtual void setPrev(BPNode<T, way>* newPrev) = 0;
        virtual void mergeLeftHere(BPNode<T, way>* dyingNode) = 0;
        virtual void mergeRightHere(BPNode<T, way>* dyingNode) = 0;
        virtual BPNode<T, way>* backSteal() = 0;
        virtual BPNode<T, way>* frontSteal() = 0;
        virtual ItemInterface* giveUpLastItem() = 0;
        virtual ItemInterface* giveUpFirstItem() = 0;
        virtual void receiveItem(ItemInterface* newItem) = 0;
        virtual bool isWealthy() = 0;
        virtual T getHardLeft() = 0;
        virtual BPNode<T, way>* overthrowRoot() = 0;
        virtual int getNumChildren() = 0;
        virtual ~BPNode() = default;
        virtual T viewSign1() = 0;
        virtual T getSign1() = 0;
        virtual void makeRoot() = 0;
        virtual void notRoot() = 0;
        virtual bool isRoot() = 0;
        virtual BPNode<T, way>* insert(ItemInterface* newItem) = 0;
        virtual RemovalResult<T> remove(T deleteIt, BPNode<T, way>* leftSibling, BPNode<T, way>* rightSibling) = 0;
        virtual ItemInterface* singleKeySearch(T findIt) = 0;
        virtual bool isLeaf() = 0;
        virtual void print(int depth) = 0;
        virtual void ripPrint(int depth) = 0;
};

#endif