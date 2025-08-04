#include <iostream>
#include "../BPlusTree.h"
#include "../Item.h"
#include "../NCItem.h"


using namespace std;

#define MILLION 1000000

void testInsertAsc() {
    cout << "Test 1: Ascending Insert" << endl;
    
    auto bpt = createBPlusTree<int>(3, 0, 272);
    
    for (int i = 0; i < 25; i++) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        cout << "Insert " << i << endl;
        bpt->insert(item);
        bpt->print();
        cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;
    }

    ItemInterface* result1 = bpt->singleKeySearch(13);
    result1->print();
    ItemInterface* result2 = bpt->singleKeySearch(14);
    result2->print();

    bpt->print();
    cout << endl;
}


void testInsertDesc() {
    cout << "Test 2: Descending Insert" << endl;
    auto bpt = createBPlusTree<int>(3, 0, 272);
    
    for (int i = 24; i >= 0; i--) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        cout << "Insert " << i << endl;
        bpt->insert(item);
        bpt->print();
        cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;
    }


    ItemInterface* result1 = bpt->singleKeySearch(13);
    result1->print();
    ItemInterface* result2 = bpt->singleKeySearch(14);
    result2->print();
    // TODO print for Items

    bpt->print();
    cout << endl;
}


void testMillionInsert() {
    cout << "Very Large Ascending Insert" << endl;
    
    auto bpt = createBPlusTree<int>(100, 0);
    
    for (int i = 0; i < MILLION; i++) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        bpt->insert(item);
    }

    bpt->print();
    cout << endl;
}


void testInsertRand() {
    cout << "Test 3: Small Pseudorandom Insert" << endl;
    
    auto bpt = createBPlusTree<int>(3, 0, 70);

    ItemInterface* item = new Item(5, {{}, {}, {}});
    bpt->insert(item);

    ItemInterface* item1 = new Item(9, {{}, {}, {}});
    bpt->insert(item1);

    ItemInterface* item2 = new Item(7, {{"hey now"}, {"ur an all star"}, {"get ur game on"}});
    bpt->insert(item2);

    ItemInterface* item3 = new Item(20, {{}, {}, {}});
    bpt->insert(item3);

    ItemInterface* item4 = new Item(1, {{}, {}, {}});
    bpt->insert(item4);

    ItemInterface* item5 = new Item(2, {{}, {}, {}});
    bpt->insert(item5);

    ItemInterface* item6 = new Item(4, {{}, {}, {}});
    bpt->insert(item6);

    ItemInterface* resultBad = bpt->singleKeySearch(3);

    ItemInterface* resultFound = bpt->singleKeySearch(7);
    // cout << "found: " << resultFound[0] << endl;
    resultFound->print();

    bpt->print();
    cout << endl;
}

void testDuplicateInsert() {
    cout << "Test 3: Duplicate Keyed Insert. Should throw error" << endl;
    
    // BPlusTree<int> bpt(3, 0, 170);
    auto bpt = createBPlusTree<int>(3, 0, 170);

    ItemInterface* item = new Item(5, {{}, {}, {}});
    bpt->insert(item);

    ItemInterface* item1 = new Item(5, {{}, {}, {}});
    bpt->insert(item1);

    bpt->print();
    cout << endl;
}




void testStringTree() {
    cout << "Test 4: Main tree and string tree" << endl;
    
    auto bpt = createBPlusTree<int>(3, 0, 272);
    auto bptAttr = createBPlusTree<AttributeType>(3, 1, 272);

    ItemInterface* item = new Item(5, {{"test"}, {"asdasd"}, {"jkljkljkl"}});
    bpt->insert(item);
    ItemInterface* nc = new NCItem(item->getPrimaryKey(), bpt);
    bptAttr->insert(nc);


    ItemInterface* item1 = new Item(9, {{"number15"}, {"plastic bins"}, {"of lettuce"}});
    bpt->insert(item1);
    ItemInterface* nc1 = new NCItem(item1->getPrimaryKey(), bpt);
    bptAttr->insert(nc1);

    ItemInterface* item2 = new Item(7, {{"hey now"}, {"ur an all star"}, {"get ur game on"}});
    bpt->insert(item2);
    ItemInterface* nc2 = new NCItem(item2->getPrimaryKey(), bpt);
    bptAttr->insert(nc2);


    ItemInterface* item3 = new Item(20, {{"hey now"}, {"what1"}, {"what2"}});
    bpt->insert(item3);
    ItemInterface* nc3 = new NCItem(item3->getPrimaryKey(), bpt);
    bptAttr->insert(nc3);


    ItemInterface* item4 = new Item(1, {{"the quick"}, {"brown fox"}, {"jumped over"}});
    bpt->insert(item4);
    ItemInterface* nc4 = new NCItem(item4->getPrimaryKey(), bpt);
    bptAttr->insert(nc4);


    ItemInterface* item5 = new Item(2, {{"aa"}, {"ab"}, {"ac"}});
    bpt->insert(item5);
    ItemInterface* nc5 = new NCItem(item5->getPrimaryKey(), bpt);
    bptAttr->insert(nc5);


    ItemInterface* item6 = new Item(4, {{"xx"}, {"xy"}, {"xz"}});
    bpt->insert(item6);
    ItemInterface* nc6 = new NCItem(item6->getPrimaryKey(), bpt);
    bptAttr->insert(nc6);


    AttributeType searchKey = {};
    std::strncpy(searchKey.data(), "hey now", searchKey.size() - 1);

    ItemInterface* foundItem = bptAttr->singleKeySearch(searchKey);

    foundItem->print();

    bpt->print();
    cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;
    bptAttr->print();
}



int main() {
    // testMillionInsert();
    // testInsertAsc();
    // testInsertDesc();
    // testInsertRand();
    // testDuplicateInsert();
    testStringTree();

    // BPLeaf<int> leaf(3, 0);
    // cout << "size of leaf with no items: " << leaf.size() << endl;
    // cout << "size of leaf with no items 2nd opinion: " << sizeof(leaf) << endl;

    // ItemInterface* item = new Item(5, {{}});
    // leaf.insert(item);

    // ItemInterface* item2 = new Item(6, {{}, {}, {}});
    // leaf.insert(item2);

    // ItemInterface* item3 = new Item(7, {{}, {}, {}});
    // leaf.insert(item3);

    // cout << "size of leaf with 3 items: " << leaf.size() << endl;
    // cout << "size of items with no attributes: " << sizeof(*item) << endl;

    return 0;
}