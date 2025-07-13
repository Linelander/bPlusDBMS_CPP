#include <iostream>
#include "../BPlusTree.h"

using namespace std;

void test1();

int main() {
    test1();
}

void test1() {
    BPlusTree bpt(3);
    
    for (int i = 0; i < 10; ++i) {
        Item item = Item(i, {}, {}, {});
        bpt.insert(item);
    }

    bpt.print();
}