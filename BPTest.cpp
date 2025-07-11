#include <iostream>
#include "BPlusTree.h"

using namespace std;

void test1();


int main() {
    test1();
}

void test1() {
    BPlusTree *bpt = new BPlusTree(3);
    
    Item *iZero = new Item(0, "0");
    bpt->insert(iZero);
    




    
    
    // bool result = ...   
    // cout << "Test 1: " + result;
}

;