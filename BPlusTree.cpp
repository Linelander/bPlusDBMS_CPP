#include "Item.h"
#include "BPlusTree.h"
#include <cstddef>
#include <vector>

using namespace std;

// BPNode* root{};
// size_t pageSize{4096};


BPlusTree::BPlusTree(int way) {
    root = new BPLeaf(way);
    root->makeRoot();
}

BPlusTree::BPlusTree(int way, size_t nonstandardSize) {
    pageSize = nonstandardSize;
    root = new BPLeaf(way, nonstandardSize);
    root->makeRoot();
}

void BPlusTree::insert(Item newItem) {
    BPNode* result = root->insert(newItem);
    if (result != NULL)
    {
        root = result;
    }
}


int BPlusTree::remove(int deleteIt) {
    
}

vector<Item> BPlusTree::search(int findIt) {

}

void BPlusTree::print() {
    root->print(0);
}

int BPlusTree::getDepth() {
    return root->getDepth(1);
}