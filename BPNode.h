#include <list>
#include "Item.h"
#include "BPKey.h"
using namespace std;

#ifndef BPNODE_H
#define BPNODE_H

class BPNode {
    public:
        virtual BPKey getKey() = 0;
        virtual int getWay() = 0;
        virtual int del() = 0;
        virtual int insert() = 0;
        virtual list<Item> search() = 0;
        virtual void setWay(int way) = 0;

};

#endif