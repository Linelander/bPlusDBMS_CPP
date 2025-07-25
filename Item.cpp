#include <cstddef>
#include <cstring>
#include<iostream>
#include "Item.h"
#include <iterator>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

using namespace std;



// METHODS
Item::Item() {}

Item::Item(int primary, vector<AttributeType> attr) {
    this->primaryKey = primary;
    this->attributes = attr;
}

int Item::getPrimaryKey() {
    return this->primaryKey;
}

AttributeType Item::getKeyByIndex(int index) {
    return attributes[index];
}

void Item::setAttributeByIndex(int index, AttributeType attr) {
    attributes[index] = attr;
}

size_t Item::size() {
    size_t result = 0;
    result += sizeof(this); // Suspicious?
    for (AttributeType a : attributes) {
        result += sizeof(a);
    }
    return result;
}

/*
this < that: return -1
this == that: return 0
this > that: return 1
*/
int Item::comparePrimary(ItemInterface* that) {
    if (this->primaryKey == that->getPrimaryKey()) {
        return 0;
    }
    if (this->primaryKey > that->getPrimaryKey()) {
        return 1;
    }
    return -1;
}

int Item::compareByIndex(ItemInterface* that, int index) {
    if (this->attributes[index] == that->getKeyByIndex(index)) {
        return 0;
    }
    if (this->attributes[index] > that->getKeyByIndex(index)) {
        return 1;
    }
    return -1;
}

int Item::dynamicCompare(ItemInterface* that, int index) {
    if (index == 0) {
        return comparePrimary(that);
    }
    return compareByIndex(that, index-1);
}

template <typename T>
int Item::comparePrimaryToKey(T thatPK) {
    if (this->primaryKey == thatPK) {
        return 0;
    }
    if (this->primaryKey > thatPK) {
        return 1;
    }
    return -1;
}

template <typename T>
int Item::compareToKeyByIndex(T that, int index) {
    if (this->attributes[index] == that) {
        return 0;
    }
    if (this->attributes[index] > that) {
        return 1;
    }
    return 0;
}

template <typename T>
int Item::dynamicCompareToKey(T that, int index) {
    if (index == 0) {
        return comparePrimaryToKey(that);
    }
    return compareToKeyByIndex(that, index);
}
