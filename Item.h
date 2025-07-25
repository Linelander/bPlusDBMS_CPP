#include <cstddef>
#include<iostream>
#include <array>
#include "ItemInterface.h"
#include <vector>

using namespace std;


#ifndef ITEM_H
#define ITEM_H

class Item : public ItemInterface {
    private:
        int primaryKey;       // 4 bytes
        vector<AttributeType> attributes;
    public:
                
        Item() {}

        Item(int primary, vector<AttributeType> attr) {
            this->primaryKey = primary;
            this->attributes = attr;
        }

        int getPrimaryKey() {
            return this->primaryKey;
        }

        AttributeType getKeyByIndex(int index) {
            return attributes[index];
        }

        void setAttributeByIndex(int index, AttributeType attr) {
            attributes[index] = attr;
        }

        size_t size() {
            size_t result = 0;
            result += sizeof(*this); // Suspicious?
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
        int comparePrimary(ItemInterface* that) {
            if (this->primaryKey == that->getPrimaryKey()) {
                return 0;
            }
            if (this->primaryKey > that->getPrimaryKey()) {
                return 1;
            }
            return -1;
        }

        int compareByIndex(ItemInterface* that, int index) {
            if (this->attributes[index] == that->getKeyByIndex(index)) {
                return 0;
            }
            if (this->attributes[index] > that->getKeyByIndex(index)) {
                return 1;
            }
            return -1;
        }

        int dynamicCompare(ItemInterface* that, int index) {
            if (index == 0) {
                return comparePrimary(that);
            }
            return compareByIndex(that, index-1);
        }

        template <typename T>
        int comparePrimaryToKey(T thatPK) {
            if (this->primaryKey == thatPK) {
                return 0;
            }
            if (this->primaryKey > thatPK) {
                return 1;
            }
            return -1;
        }

        template <typename T>
        int compareToKeyByIndex(T that, int index) {
            if (this->attributes[index] == that) {
                return 0;
            }
            if (this->attributes[index] > that) {
                return 1;
            }
            return -1;
        }

        template <typename T>
        int dynamicCompareToKey(T that, int index) {
            if (index == 0) {
                return comparePrimaryToKey(that);
            }
            return compareToKeyByIndex(that, index-1);
        }
};

// METHODS

#endif