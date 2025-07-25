#include <cstddef>
#include<iostream>
#include <array>
#include "ItemInterface.h"
#include <vector>

using namespace std;


#ifndef ITEM_H
#define ITEM_H

class Item : ItemInterface {
    public:
        Item();
        Item(int primary, vector<AttributeType> attr);
        
        int getPrimaryKey();
        AttributeType getKeyByIndex(int index);

        void setAttributeByIndex(int index, AttributeType attr);

        size_t size();

        /*
        this < that: return -1
        this == that: return 0
        this > that: return 1
        */
        int comparePrimary(ItemInterface* that);
        int compareByIndex(ItemInterface* that, int index);
        int dynamicCompare(ItemInterface* that, int index);
        
        template <typename T>
        int compareToKeyByIndex(T that, int index);
        
        template <typename T>
        int comparePrimaryToKey(T thatPK);
        
        template <typename T>
        int dynamicCompareToKey(T that, int index);

    private:
        int primaryKey;       // 4 bytes
        vector<AttributeType> attributes;
};

#endif