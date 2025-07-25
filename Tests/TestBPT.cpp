#include <iostream>
#include "../BPlusTree.h"


using namespace std;

#define MILLION 1000000

void testInsert1() {
    cout << "Test 1: Ascending Insert" << endl;
    
    BPlusTree bpt(5, 272);
    
    for (int i = 0; i < 300; i++) {
        Item item = Item(i, {}, {}, {});
        bpt.insert(item);
    }

    bpt.print();
    cout << endl;
}


void testInsert2() {
    cout << "Test 2: Descending Insert" << endl;
    BPlusTree bpt(3, 272);
    
    for (int i = 20; i >= 0; i--) {
        Item item = Item(i, {}, {}, {});
        bpt.insert(item);
    }

    bpt.print();
    cout << endl;
}


void testMillionInsert() {
    cout << "Very Large Ascending Insert" << endl;
    
    BPlusTree bpt(100);
    
    for (int i = 0; i < 20000000; i++) {
        Item item = Item(i, {}, {}, {});
        bpt.insert(item);
    }

    bpt.print();
    cout << endl;
}


void testInsert3() {
    cout << "Test 3: Small Pseudorandom Insert" << endl;
    
    BPlusTree bpt(3, 170);


    Item item = Item(5, {}, {}, {});
    bpt.insert(item);

    Item item1 = Item(9, {}, {}, {});
    bpt.insert(item1);

    Item item2 = Item(7, {}, {}, {});
    bpt.insert(item2);

    Item item3 = Item(20, {}, {}, {});
    bpt.insert(item3);

    Item item4 = Item(1, {}, {}, {});
    bpt.insert(item4);

    Item item5 = Item(2, {}, {}, {});
    bpt.insert(item5);

    Item item6 = Item(4, {}, {}, {});
    bpt.insert(item6);

    bpt.print();
    cout << endl;
}

void testDuplicateInsert() {
    cout << "Test 3: Duplicate Keyed Insert. Should throw error" << endl;
    
    BPlusTree bpt(3, 170);

    Item item = Item(5, {}, {}, {});
    bpt.insert(item);

    Item item1 = Item(5, {}, {}, {});
    bpt.insert(item1);

    bpt.print();
    cout << endl;
}






int main() {
    testMillionInsert();
    // testInsert1();
    // testInsert2();
    // testInsert3();
    // testDuplicateInsert();

    return 0;
}