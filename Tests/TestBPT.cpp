#include <iostream>
#include "../BPlusTree.h"


using namespace std;

#define MILLION 1000000

void testInsert1() {
    cout << "Test 1: Ascending Insert" << endl;
    
    BPlusTree<int> bpt(5, 0, 272);
    
    for (int i = 0; i < 300; i++) {
        ItemInterface * item = new Item(i, {{}, {}, {}});
        bpt.insert(item);
    }

    bpt.print();
    cout << endl;
}


void testInsert2() {
    cout << "Test 2: Descending Insert" << endl;
    BPlusTree<int> bpt(3, 0, 272);
    
    for (int i = 20; i >= 0; i--) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        bpt.insert(item);
    }

    bpt.print();
    cout << endl;
}


void testMillionInsert() {
    cout << "Very Large Ascending Insert" << endl;
    
    BPlusTree<int> bpt(100, 0);
    
    for (int i = 0; i < 20000000; i++) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        bpt.insert(item);
    }

    bpt.print();
    cout << endl;
}


void testInsert3() {
    cout << "Test 3: Small Pseudorandom Insert" << endl;
    
    BPlusTree<int> bpt(3, 0, 170);

    ItemInterface* item = new Item(5, {{}, {}, {}});
    bpt.insert(item);

    ItemInterface* item1 = new Item(9, {{}, {}, {}});
    bpt.insert(item1);

    ItemInterface* item2 = new Item(7, {{}, {}, {}});
    bpt.insert(item2);

    ItemInterface* item3 = new Item(20, {{}, {}, {}});
    bpt.insert(item3);

    ItemInterface* item4 = new Item(1, {{}, {}, {}});
    bpt.insert(item4);

    ItemInterface* item5 = new Item(2, {{}, {}, {}});
    bpt.insert(item5);

    ItemInterface* item6 = new Item(4, {{}, {}, {}});
    bpt.insert(item6);

    bpt.print();
    cout << endl;
}

void testDuplicateInsert() {
    cout << "Test 3: Duplicate Keyed Insert. Should throw error" << endl;
    
    BPlusTree<int> bpt(3, 0, 170);

    ItemInterface* item = new Item(5, {{}, {}, {}});
    bpt.insert(item);

    ItemInterface* item1 = new Item(5, {{}, {}, {}});
    bpt.insert(item1);

    bpt.print();
    cout << endl;
}



int main() {
    testMillionInsert();
    testInsert1();
    testInsert2();
    testInsert3();
    testDuplicateInsert();

    return 0;
}