#include <iostream>
#include "../BPlusTree.h"

using namespace std;

void test1() {
    BPlusTree bpt(3, 272);
    
    for (int i = 0; i < 20; i++) {
        Item item = Item(i, {}, {}, {});
        // if (i == 9)
        // {
        //     cout << "hi";
        // }
        bpt.insert(item);
    }

    bpt.print();
}



void test2() {
    BPlusTree bpt(3, 272);
    
    for (int i = 20; i >= 0; i--) {
        Item item = Item(i, {}, {}, {});
        // if (i == 9)
        // {
        //     cout << "hi";
        // }
        bpt.insert(item);
    }

    bpt.print();
}


int main() {
    test1();
    test2();
}

