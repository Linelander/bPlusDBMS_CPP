#include <cstring>
#include<iostream>
#include "Item.h"
#include "BPKey.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

using namespace std;


int key1;
KeyType key2;
KeyType key3;
KeyType key4;

Item::Item() {}

Item::Item(int k1, KeyType k2, KeyType k3, KeyType k4) {
    key1 = k1;
    key2 = k2;
    key3 = k3;
    key4 = k4;
}

void setKey1(int k1) {key1 = k1;}
void setKey2(KeyType k2) {key2 = k2;}
void setKey3(KeyType k3) {key3 = k3;}
void setKey4(KeyType k4) {key4 = k4;}

int getKey1()       {return key1;}
KeyType getKey2()   {return key2;}
KeyType getKey3()   {return key3;}
KeyType getKey4()   {return key4;}

string getFullKey() {
    string result = to_string(key1) + string(key2.data()) + string(key3.data()) + string(key4.data());
    return result;
}

 