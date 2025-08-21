#include <cstddef>
#include <iostream>
#include "BPNode.h"


#define DIRTY true
#define CLEAN false


using namespace std;

#ifndef NODE_PAGE
#define NODE_PAGE
template <typename T, int way>f
class NodePage {
    private:
        BPNode<T, way>* nodePtr;
        size_t pageOffset;
        bool dirtybit = DIRTY;
        bool currentlyUsing = true;
    
    public:
        
        NodePage(BPNode<T, way>* node, size_t offset) : nodePtr(node), pageOffset(offset) {}
        
        size_t getPageOffset() {return pageOffset;}
        
        void markDirty() {dirtybit = DIRTY;}
        
        void markClean() {dirtybit = CLEAN;}
        
        bool getDirty() {return dirtybit;}
        
        void release() {currentlyUsing = false;}
        
        bool getCurrentlyUsing() {return currentlyUsing;}
};
#endif