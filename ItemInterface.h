#include <iostream>

using namespace std;
using KeyType = array<char, 16>;


class ItemInterface {
        
        virtual int getKey1() = 0;
        virtual KeyType getKey2() = 0;
        virtual KeyType getKey3() = 0;
        virtual KeyType getKey4() = 0;
        virtual string getFullKey() = 0;
        virtual void setKey1(int k1) = 0;
        virtual void setKey2(KeyType k2) = 0;
        virtual void setKey3(KeyType k3) = 0;
        virtual void setKey4(KeyType k4) = 0;
        virtual size_t size() = 0;

        virtual bool comparePrimary(ItemInterface* that) = 0;
        virtual bool comparePrimary(ItemInterface* that) = 0;
};