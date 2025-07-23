#include "Item.h"
#include "BPlusTree.h"
#include <cstddef>
#include <vector>

using namespace std;

BPNode* root{};
size_t pagesize{4096};


BPlusTree::BPlusTree(int way) {
    root = new BPLeaf(way);
}

BPlusTree::BPlusTree(int way, size_t nonstandardSize) {
    pagesize = nonstandardSize;
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
    // "inorder traversal" that prints the root half-way through iterating through subtrees
    
    // if (!root->isLeaf())
    // {
    //     vector subtrees = *root->getChildren();
    //     for (int i = 0; i < subtrees.size(); i++)
    //     {
    //         if (i == subtrees.size() / 2)
    //         {
    //             root->print(0);
    //         }
    //         subtrees[i]->print(1);
    //     }
    // }
    // else {
    //     root->print(0);
    // }

    root->print(0);
}

int getDepth() {
    return root->getDepth(1);
}