#include <cstddef>
#include <cstring>
#include<iostream>
#include "Item.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

using namespace std;


// METHODS
Item::Item() {}

Item::Item(int k1, KeyType k2, KeyType k3, KeyType k4) {
    key1 = k1;
    key2 = k2;
    key3 = k3;
    key4 = k4;
}

void Item::setKey1(int k1) {key1 = k1;}
void Item::setKey2(KeyType k2) {key2 = k2;}
void Item::setKey3(KeyType k3) {key3 = k3;}
void Item::setKey4(KeyType k4) {key4 = k4;}

int Item::getKey1()       {return key1;}
KeyType Item::getKey2()   {return key2;}
KeyType Item::getKey3()   {return key3;}
KeyType Item::getKey4()   {return key4;}

string Item::getFullKey() {
    string result = to_string(key1) + string(key2.data()) + string(key3.data()) + string(key4.data());
    return result;
}

size_t Item::size()
{
    return sizeof(Item);
}

 