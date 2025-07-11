#include "Item.h"
#include "BPlusTree.h"
using namespace std;

BPNode* root{};

BPlusTree::BPlusTree(int way) {
    root = new BPLeaf(way);
}

int BPlusTree::insert(Item newItem) {

}
int del(BPKey deleteIt) {
    
}
list<Item> search(BPKey findIt) {

}