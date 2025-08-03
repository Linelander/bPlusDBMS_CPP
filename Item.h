#include <cstddef>
#include <cstring>
#include<iostream>
#include <array>
#include "ItemInterface.h"
#include <vector>
#include <cstring>
// #include <any>

using namespace std;
using KeyType = array<char, 16>;


#ifndef ITEM_H
#define ITEM_H

class Item : public ItemInterface {
    private:
        int primaryKey;       // 4 bytes. max value: 99,999,999
        vector<AttributeType> attributes;
    public:
        ~Item() {}
    
        Item() {}

        Item(int primary, vector<AttributeType> attr) {
            this->primaryKey = primary;
            this->attributes = attr;
        }

        vector<ItemInterface*> singleKeySearchResult() {
            return {this};
        }

        void addDupeKey(int newDupe) {
            cout << "ERROR: Class Item is used for clustered indexes on the primary key and does not support duplicates." << endl;
        }


        int getPrimaryKey() {
            return this->primaryKey;
        }

        AttributeType getKeyByIndex(int index) {
            return attributes[index];
        }

        const any dynamicGetKeyByIndex(int index) {
            if (index == 0)
            {
                return this->primaryKey;
            }
            return attributes[index-1];
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
            cout << "Size of regular item: " << result << endl;
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

        // template <typename T>
        int comparePrimaryToKey(const any& thatPK) {
            int castThatPK = any_cast<int>(thatPK);
            if (this->primaryKey == castThatPK) {
                return 0;
            }
            if (this->primaryKey > castThatPK) {
                return 1;
            }
            return -1;
        }

        // Compares attributes. Not to be used on primary keys.
        int compareToKeyByIndex(const any& thatK, int index) {
            AttributeType castThatK = any_cast<AttributeType>(thatK);
            if (this->attributes[index] == castThatK) {
                return 0;
            }
            if (this->attributes[index] > castThatK) {
                return 1;
            }
            return -1;
        }

        // Routes comparisons on index 0 to the comparator for primary keys and casts the key as an int
        // Routes comparisons on all other indexes to the attribute comparator compareToKeyByIndex
        int dynamicCompareToKey(const any& thatK, int index) {
            if (index == 0) {
                return comparePrimaryToKey(thatK);
            }
            return compareToKeyByIndex(thatK, index-1);
        }
};

// METHODS

#endif