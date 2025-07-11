#include "Item.h"
#include "BPlusTree.h"
#include <vector>

using namespace std;

BPNode* root{};

BPlusTree::BPlusTree(int way) {
    root = new BPLeaf(way);
}

int BPlusTree::insert(Item newItem) {
    
}
int del(BPKey deleteIt) {
    
}
vector<Item> search(BPKey findIt) {

}

void print(int depth) {
    // "inorder traversal" that prints the root half-way through iterating through subtrees
    
    if (!root->isLeaf())
    {
        vector subtrees = root->getChildren();
        for (int i = 0; i < subtrees.size(); i++)
        {
            if (i == subtrees.size() / 2)
            {
                root->print(0);
            }
            subtrees[i].print(1);
        }
    }
    else {
        root->print(0);
    }

    
    
}

int getDepth() {
    root->getDepth(0);
}