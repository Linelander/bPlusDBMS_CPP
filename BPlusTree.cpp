#include "Item.h"
#include "BPlusTree.h"
#include <cstddef>
#include <vector>

using namespace std;


template <typename T>
BPlusTree<T>::BPlusTree(int way, int keyIndex) {
    root = new BPLeaf<T>(way);
    root->makeRoot();
}

template <typename T>
BPlusTree<T>::BPlusTree(int way, int keyIndex, size_t nonstandardSize) {
    pageSize = nonstandardSize;
    root = new BPLeaf<T>(way, nonstandardSize);
    root->makeRoot();
}

template <typename T>
void BPlusTree<T>::insert(Item newItem) {
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
vector<Item> BPlusTree<T>::search(int findIt) {

}

template <typename T>
void BPlusTree<T>::print() {
    root->print(0);
}

template <typename T>
int BPlusTree<T>::getDepth() {
    return root->getDepth(1);
}