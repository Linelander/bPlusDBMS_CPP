#include "BPlusTree.h"
using namespace std;

BPNode* root{};

BPlusTree::BPlusTree(int way) {
    root = new BPLeaf(way);
}
;