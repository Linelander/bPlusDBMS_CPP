#include <iostream>
#include "../BPlusTree.h"

using namespace std;

void test1() {
    BPlusTree bpt(3, 272);
    
    for (int i = 0; i < 20; i++) {
        Item item = Item(i, {}, {}, {});
        bpt.insert(item);
    }

    bpt.print();
}


// void testLeafSplit1() {
//     cout << "test 1: ascending insertion" << endl;
//     BPLeaf fourWay = BPLeaf(4, 272);

//     for (int i = 0; i < 5; i++)
//     {
//         fourWay.insert(Item(i, {'_'}, {'_'}, {'_'}));
//     }

//     fourWay.print(0);
//     fourWay.getNeighbor()->print(0);

//     cout << endl;
// }


// void testLeafSplit2() {
//     cout << "test 2: descending insertion" << endl;
//     BPLeaf fourWay = BPLeaf(4, 272);

//     for (int i = 4; i >= 0; i--)
//     {
//         fourWay.insert(Item(i, {'_'}, {'_'}, {'_'}));
//     }

//     fourWay.print(0);
//     fourWay.getNeighbor()->print(0);

//     cout << endl;
// }

int main() {
    // testLeafSplit1();
    // testLeafSplit2();
    test1();
}

