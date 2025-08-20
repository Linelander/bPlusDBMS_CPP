#include <iostream>
#include "../BPlusTree.h"
#include "../Item.h"
#include "../NCItem.h"


using namespace std;

#define MILLION 1000000

void testInsertAsc() {
    cout << "Test 1: Ascending Insert" << endl;
    
    auto bpt = createBPlusTree<int>(3, 0, "bpt", "bptcol", 272);
    
    for (int i = 0; i < 200; i++) {
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
    auto bpt = createBPlusTree<int>(3, 0, "bpt", "bptcol", 272);
    
    for (int i = 24; i >= 0; i--) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        // cout << "Insert " << i << endl;
        bpt->insert(item);
        // bpt->print();
        // cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;
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
    
    auto bpt = createBPlusTree<int>(100, 0, "bpt", "bptcol");
    
    for (int i = 0; i < MILLION; i++) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        bpt->insert(item);
    }

    bpt->print();
    cout << endl;
}


void testInsertRand() {
    cout << "Test 3: Small Pseudorandom Insert" << endl;
    
    auto bpt = createBPlusTree<int>(3, 0, "bpt", "bptcol", 70);

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
    auto bpt = createBPlusTree<int>(3, 0, "bpt", "bptcol", 170);

    ItemInterface* item = new Item(5, {{}, {}, {}});
    bpt->insert(item);

    ItemInterface* item1 = new Item(5, {{}, {}, {}});
    bpt->insert(item1);

    bpt->print();
    cout << endl;
}




void testStringTree() {
    cout << "Test 4: Main tree and string tree" << endl;
    
    auto bpt = createBPlusTree<int>(3, 0, "bpt", "bptcol", 272);
    auto bptAttr = createBPlusTree<AttributeType>(3, 1, "attribute bpt", "bptcol2", 272);

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

    // bpt->print();
    cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;
    // bptAttr->print();
}


void testRemove() {
    cout << "Test 5: Remove on int and string tree" << endl;
    
    auto bpt = createBPlusTree<int>(3, 0, "bpt", "bptcol", 272);
    auto bptAttr = createBPlusTree<AttributeType>(3, 1, "attribute bpt", "bptcol2", 272);

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

    bpt->print();
    bptAttr->print();


    ItemInterface* foundIntItem = bpt->singleKeySearch(4);
    foundIntItem->print();

    cout << "------------------" << endl;

    AttributeType searchKey = {};
    std::strncpy(searchKey.data(), "hey now", searchKey.size() - 1);
    ItemInterface* foundItem = bptAttr->singleKeySearch(searchKey);
    foundItem->print();
    

    cout << "-----------------------------------------------------DELETE---------------------------------------------------------------" << endl;

    bpt->remove(4);
    ItemInterface* goneIntItem = bpt->singleKeySearch(4);
    cout << "after deleting 4:" << endl;
    // goneIntItem->print();
    bpt->print();
}


void testRemove2() {
    cout << "Remove Test 2: After Ascending Insert" << endl;
    
    auto bpt = createBPlusTree<int>(3, 0, "bpt", "bptcol", 272);
    
    for (int i = 0; i < 25; i++) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        cout << "Insert " << i << endl;
        bpt->insert(item);
        // bpt->print();
        // cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;
    }

    ItemInterface* result14 = bpt->singleKeySearch(14);
    result14->print();

    bpt->print();
    cout << endl;

    cout << "-----------------------------------------------------DELETE---------------------------------------------------------------" << endl;

    bpt->remove(14);

    bpt->print();
    cout << endl;

    ItemInterface* goneResult14 = bpt->singleKeySearch(14);
}


void testRemove3() {
    cout << "Remove Test 3: After Descending Insert" << endl;
    auto bpt = createBPlusTree<int>(3, 0, "bpt", "bptcol", 272);
    
    for (int i = 24; i >= 0; i--) {
        ItemInterface* item = new Item(i, {{}, {}, {}});
        // cout << "Insert " << i << endl;
        bpt->insert(item);
        // bpt->print();
        // cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;
    }


    ItemInterface* result1 = bpt->singleKeySearch(13);
    result1->print();
    ItemInterface* result2 = bpt->singleKeySearch(14);
    result2->print();
    // TODO print for Items

    bpt->print();
    cout << endl;

    cout << "============ FIRST RIP PRINT ============" << endl;
    bpt->ripPrint();

    cout << "-----------------------------------------------------DELETE---------------------------------------------------------------" << endl;

    // Delete 6, 8, 10, 12, then 5

    cout << "deleting 6" << endl;
    bpt->remove(6);
    bpt->print();
    cout << endl;

    cout << "deleting 8" << endl;
    bpt->remove(8);
    bpt->print();
    cout << endl;

    cout << "deleting 10" << endl;
    bpt->remove(10);
    bpt->print();
    cout << endl;

    cout << "deleting 12" << endl;
    bpt->remove(12);
    bpt->print();
    cout << endl;

    cout << "deleting 4" << endl;
    bpt->remove(4);
    bpt->print();
    cout << endl;
    
    // merge time
    cout << "deleting 5" << endl;
    bpt->remove(5);
    bpt->print();
    cout << endl;

    cout << "deleting 3" << endl;
    bpt->remove(3);
    bpt->print();
    cout << endl;

    cout << "deleting 24" << endl;
    bpt->remove(24);
    bpt->print();

    cout << "deleting 23" << endl;
    bpt->remove(23);
    bpt->print();

    cout << "deleting 17" << endl;
    bpt->remove(17);
    bpt->print();

    cout << "deleting 13" << endl;
    bpt->remove(13);
    bpt->print();

    cout << "deleting 14" << endl;
    bpt->remove(14);
    bpt->print();

    cout << "deleting 9" << endl;
    bpt->remove(9);
    bpt->print();

    cout << "deleting 19" << endl;
    bpt->remove(19);
    bpt->print();

    cout << "deleting 22" << endl;
    bpt->remove(22);
    bpt->print();

    cout << "deleting 21" << endl;
    bpt->remove(21);
    bpt->print();

    cout << "deleting 15" << endl;
    bpt->remove(15);
    bpt->print();

    cout << "deleting 16" << endl;
    bpt->remove(16);
    bpt->print();

    cout << endl << "================== RIP PRINT ==================" << endl;
    bpt->ripPrint();

    cout << "deleting 2" << endl;
    bpt->remove(2);
    bpt->print();

    cout << "deleting 0" << endl;
    bpt->remove(0);
    bpt->print();

    cout << "deleting 1" << endl;
    bpt->remove(1);
    bpt->print();

    cout << "deleting 20" << endl;
    bpt->remove(20);
    bpt->print();

    cout << "deleting 18" << endl;
    bpt->remove(18);
    bpt->print();

    cout << "deleting 7" << endl;
    bpt->remove(7);
    bpt->print();

    cout << "deleting 11" << endl;
    bpt->remove(11);
    bpt->print();
}


int main() {
    // testMillionInsert();
    testInsertAsc();
    // testInsertDesc();
    // testInsertRand();
    // testDuplicateInsert();
    // testStringTree();
    // testRemove();
    // testRemove2();
    // testRemove3();

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