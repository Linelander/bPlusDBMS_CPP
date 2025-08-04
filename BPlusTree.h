#include <cstddef>
#include<iostream>
#include "BPNode.h"
#include <vector>
#include <memory>
#include <stdexcept>


using namespace std;

// template<typename T> class BPInternalNode;
// template<typename T> class BPLeaf;

#ifndef BP_TREE
#define BP_TREE




template <typename T>
class BPlusTreeBase {
    public:
        virtual ~BPlusTreeBase() = default;
        virtual void insert(ItemInterface* newItem) = 0;
        virtual ItemInterface* remove(T deleteIt) = 0;
        virtual void print() = 0;
        virtual ItemInterface* singleKeySearch(T findIt) = 0;
        virtual int getDepth() = 0;
};



template <typename T, int way>
class BPlusTree : public BPlusTreeBase<T> {
    private:
        int itemKeyIndex;
        BPNode<T>* root{};
        size_t pageSize = 4096;


        
    public:
        ~BPlusTree();
        BPlusTree(int keyIndex);
        BPlusTree(int keyIndex, size_t nonstandardSize);
        void insert(ItemInterface* newItem);
        ItemInterface* remove(T deleteIt);
        ItemInterface* singleKeySearch(T findIt);
        void print();
        int getDepth();
};

#include "BPLeaf.h"
template <typename T, int way>
BPlusTree<T, way>::BPlusTree(int keyIndex) {
    root = new BPLeaf<T, way>(keyIndex);
    root->makeRoot();
}

// Real class
template <typename T, int way>
BPlusTree<T, way>::BPlusTree(int keyIndex, size_t nonstandardSize) {
    pageSize = nonstandardSize;
    root = new BPLeaf<T, way>(keyIndex, nonstandardSize);
    root->makeRoot();
}

template <typename T, int way>
BPlusTree<T, way>::~BPlusTree() {
    delete root;
}

template <typename T, int way>
ItemInterface* BPlusTree<T, way>::singleKeySearch(T findIt) {
    return root->singleKeySearch(findIt);
}

template <typename T, int way>
void BPlusTree<T, way>::insert(ItemInterface* newItem) {
    BPNode<T>* result = root->insert(newItem);
    if (result != NULL)
    {
        root = result;
    }
}

template <typename T, int way>
ItemInterface* BPlusTree<T, way>::remove(T deleteIt) {
    
}

template <typename T, int way>
void BPlusTree<T, way>::print() {
    root->print(0);
}

template <typename T, int way>
int BPlusTree<T, way>::getDepth() {
    return root->getDepth(1);
}

// FACTORIES
template<typename T>
std::shared_ptr<BPlusTreeBase<T>> createBPlusTree(int way, int keyIndex) {
    switch(way) {
        case 3: return std::make_unique<BPlusTree<T, 3>>(keyIndex);
        case 5: return std::make_unique<BPlusTree<T, 5>>(keyIndex);
        case 8: return std::make_unique<BPlusTree<T, 8>>(keyIndex);
        case 16: return std::make_unique<BPlusTree<T, 16>>(keyIndex);
        case 100: return std::make_unique<BPlusTree<T, 100>>(keyIndex);
        case 128: return std::make_unique<BPlusTree<T, 128>>(keyIndex);
        default: 
            throw std::invalid_argument("Bad way value: " + std::to_string(way));
    }
}

template<typename T>
std::shared_ptr<BPlusTreeBase<T>> createBPlusTree(int way, int keyIndex, size_t pageSize) {
    switch(way) {
        case 3: return std::make_unique<BPlusTree<T, 3>>(keyIndex, pageSize);
        case 5: return std::make_unique<BPlusTree<T, 5>>(keyIndex, pageSize);
        case 8: return std::make_unique<BPlusTree<T, 8>>(keyIndex, pageSize);
        case 16: return std::make_unique<BPlusTree<T, 16>>(keyIndex, pageSize);
        case 100: return std::make_unique<BPlusTree<T, 100>>(keyIndex, pageSize);
        case 128: return std::make_unique<BPlusTree<T, 128>>(keyIndex, pageSize);
        default: 
            throw std::invalid_argument("Bad way value: " + std::to_string(way));
    }
}

#endif