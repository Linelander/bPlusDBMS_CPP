#include <iostream>
#include "../BPLeaf.h"

using namespace std;

void testSplit1() {
    cout << "test 1: ascending insertion" << endl;
    BPLeaf fourWay = BPLeaf(4, 272);

    for (int i = 0; i < 5; i++)
    {
        fourWay.insert(Item(i, {'_'}, {'_'}, {'_'}));
    }

    fourWay.print(0);
    fourWay.getNeighbor()->print(0);

    cout << endl;
}


void testSplit2() {
    cout << "test 2: descending insertion" << endl;
    BPLeaf fourWay = BPLeaf(4, 272);

    for (int i = 4; i >= 0; i--)
    {
        fourWay.insert(Item(i, {'_'}, {'_'}, {'_'}));
    }

    fourWay.print(0);
    fourWay.getNeighbor()->print(0);

    cout << endl;
}


int main()
{
    testSplit1();
    testSplit2();
    return 0;
}