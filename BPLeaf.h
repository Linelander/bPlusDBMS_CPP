#include<iostream>
#include "Item.h"
#include <list>
#include "BPKey.h"
using namespace std;


class BPLeaf {
    public:
        int getKey(); // What type should keys be?
        int getWay();
        bool atLeastHalfFull();
        int insert(Item);
        int del(Item);
        int search(BPKey);


    private:
        int key;
        int way{};
        list<Item> items;
        BPLeaf* overflow = NULL;

};