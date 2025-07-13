#include <iostream>
#include "../BPLeaf.h"

using namespace std;

void testSplit() {
    BPLeaf fourWay = BPLeaf(4, 272);

    for (int i = 0; i < 5; i++)
    {
        fourWay.insert(Item(i, {'_'}, {'_'}, {'_'}));
    }

    fourWay.print(0);
    fourWay.getNeighbor()->print(0);
}

int main()
{
    testSplit();
    return 0;
}