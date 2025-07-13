

#include <cstddef>
#include<iostream>
#include <array>

using namespace std;
using KeyType = array<char, 16>;

#ifndef ITEM_H
#define ITEM_H

class Item {
    public:
        Item();
        Item(int k1, KeyType k2, KeyType k3, KeyType k4);
        
        int getKey1();
        KeyType getKey2();
        KeyType getKey3();
        KeyType getKey4();

        string getFullKey();
        
        void setKey1(int k1);
        void setKey2(KeyType k2);
        void setKey3(KeyType k3);
        void setKey4(KeyType k4);

        size_t size();

    private:
        int key1;       // 4 bytes
        KeyType key2;  // 16 bytes
        KeyType key3;  // 16 bytes
        KeyType key4;  // 16 bytes
};

#endif