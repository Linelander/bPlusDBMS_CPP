#include <iostream>
#include "BPlusTree.h"

using namespace std;

void test1();


int main() {
    test1();
}

void test1() {
    BPlusTree bpt(3);
    
    for (int i = 0; i < 10; ++i) {
        Item item(i, std::to_string(i));
        bpt.insert(item);
    }

    // bpt.print();


    




    
    
    // bool result = ...   
    // cout << "Test 1: " + result;
}

;