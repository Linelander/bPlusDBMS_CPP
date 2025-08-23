
#ifndef BP_ITEM_INTERFACE
#define BP_ITEM_INTERFACE

#include <iostream>
#include <any>
#include <vector>
#include <array>
#include <cstdint>

#define COLUMN_LENGTH 16

using namespace std;
using AttributeType = array<char, COLUMN_LENGTH>;

class ItemInterface {
        public:
                virtual ~ItemInterface () = default;
                virtual vector<ItemInterface*> singleKeySearchResult() = 0;
                virtual vector<uint8_t> getBytes() = 0;
                virtual int getPrimaryKey() = 0;
                virtual AttributeType getKeyByIndex(int index) = 0;
                virtual const any dynamicGetKeyByIndex(int index) = 0;
                virtual void addDupeKey(int newDupe) = 0;
                // need to be able to remove dupe keys too.

                virtual bool isClustered() = 0;
                virtual void removeAll() = 0;


                virtual void setAttributeByIndex(int index, AttributeType attr) = 0;

                virtual size_t size() = 0;
                
                /*
                this < that: return -1
                this == that: return 0
                this > that: return 1
                */
                virtual int comparePrimary(ItemInterface* that) = 0;
                virtual int compareByIndex(ItemInterface* that, int index) = 0;
                virtual int dynamicCompare(ItemInterface* that, int index) = 0;
                
                virtual int comparePrimaryToKey(const any& thatPK) = 0;
                
                virtual int compareToKeyByIndex(const any& thatK, int index) = 0;
                
                virtual int dynamicCompareToKey(const any& thatPK, int index) = 0;

                virtual void print() = 0;
};

#endif