#include <iostream>

using namespace std;
using AttributeType = array<char, 16>;

#ifndef BP_ITEM_INTERFACE
#define BP_ITEM_INTERFACE

class ItemInterface {
        public:
                virtual int getPrimaryKey() = 0;
                virtual AttributeType getKeyByIndex(int index) = 0;

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
};

#endif