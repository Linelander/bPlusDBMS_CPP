#include<iostream>
#include "BPKey.h"
using namespace std;

#ifndef ITEM_H
#define ITEM_H

class Item {
    public:
        Item(int value, string initKey);
        BPKey getKey();
    
    private:
        BPKey key;
        int value; // Placeholder
};

#endif