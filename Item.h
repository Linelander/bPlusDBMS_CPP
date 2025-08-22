#include <cstddef>
#include <cstring>
#include<iostream>
#include <array>
#include "ItemInterface.h"
#include <stdexcept>
#include <vector>
#include <cstring>
#include <stdint.h>
// #include <any>


using namespace std;


#ifndef ITEM_H
#define ITEM_H

class Item : public ItemInterface {
    private:
        int primaryKey;       // 4 bytes. TODO max value: 99,999,999
        vector<AttributeType> attributes;
    public:

        // METHODS

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

        bool isClustered() {return true;}

        void removeAll() {throw std::runtime_error("tried to do REMOVE ALL WHERE on clustered index");}

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
            return sizeof(primaryKey) + (sizeof(AttributeType) * attributes.size());
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

        void print() {
            cout << "| ";
            for (AttributeType att : attributes)
            {
                for (int i = 0; i < COLUMN_LENGTH; i++)
                {
                    if (att[i])
                    {
                        cout << att[i];
                    }
                    else {cout << " ";}
                }
                cout << " | ";
            }
            cout << endl;
        }

        // DISK

        // Return this Item's bytes
        /*
        Format:

        - Primary key (4 bytes)
        - Then columns (16 bytes each)

        */
        vector<uint8_t> getBytes() {
            std::vector<uint8_t> bytes;
            
            // 4-byte header
            // Primary Key (4 bytes)
            bytes.push_back(static_cast<uint8_t>(primaryKey));


            for (const AttributeType& column : attributes) {
                for (int i = 0; i < COLUMN_LENGTH; i++) {
                    bytes.push_back(static_cast<uint8_t>(column[i]));
                }
            }

            return bytes;
        }
};

#endif