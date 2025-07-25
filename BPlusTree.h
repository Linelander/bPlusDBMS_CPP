#include <cstddef>
#include<iostream>
#include "BPNode.h"
#include <vector>

using namespace std;

// template<typename T> class BPInternalNode;
// template<typename T> class BPLeaf;

#ifndef BP_TREE
#define BP_TREE

template <typename T>
class BPlusTree {
    public:
        BPlusTree(int way, int keyIndex);
        BPlusTree(int way, int keyIndex, size_t nonstandardSize);
        void insert(ItemInterface* newItem);
        int remove(int deleteIt);
        vector<ItemInterface*> search(int findIt);
        void print();
        int getDepth();

    private:
        int itemKeyIndex;
        BPNode<T>* root{};
        size_t pageSize = 4096;
};

#include "BPLeaf.h"
template <typename T>
BPlusTree<T>::BPlusTree(int way, int keyIndex) {
    root = new BPLeaf<T>(way, keyIndex);
    root->makeRoot();
}

template <typename T>
BPlusTree<T>::BPlusTree(int way, int keyIndex, size_t nonstandardSize) {
    pageSize = nonstandardSize;
    root = new BPLeaf<T>(way, keyIndex, nonstandardSize);
    root->makeRoot();
}

template <typename T>
void BPlusTree<T>::insert(ItemInterface* newItem) {
    BPNode<T>* result = root->insert(newItem);
    if (result != NULL)
    {
        root = result;
    }
}


template <typename T>
int BPlusTree<T>::remove(int deleteIt) {
    
}

template <typename T>
vector<ItemInterface*> BPlusTree<T>::search(int findIt) {

}

template <typename T>
void BPlusTree<T>::print() {
    root->print(0);
}

template <typename T>
int BPlusTree<T>::getDepth() {
    return root->getDepth(1);
}

#endif