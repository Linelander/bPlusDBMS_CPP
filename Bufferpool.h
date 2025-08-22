/*

LRU Bufferpool. Allocates disk space with a freelist.

*/

#include <cstddef>
#include <iostream>

#include "Freelist.h"
#include "NodePage.h"

using namespace std;

#ifndef BUFFER_POOL
#define BUFFER_POOL
template <typename T, int way>
class Bufferpool {
    private:
        Freelist* freelist;
        int fd;
        NodePage<T, way>* root;
        vector<NodePage<T, way>*> nodePages; // More recently used pages go to the end of the vector. Using LRU.
                                                // remember to limit size

    public:
        Bufferpool(size_t pageSize, int file) : fd(file) {
            freelist = new Freelist(pageSize);
        }

        NodePage<T, way>* getPage(size_t pageOffset) {

        }

        BPNode<T, way>* getNode(size_t pageOffset) {

        }
        
        NodePage<T, way>* allocate(BPNode<T, way>* newNode) {
            size_t offset = freelist->allocate();
            NodePage<T, way> newPage = new NodePage<T, way>(newNode, offset);
            // Then we put it in the buffer... need to put it in a meaningful spot.
            // Maybe implement pins and sort the buffer based on pins
        }
        
        void markDirty(size_t pageOffset) {
            int i = 0;
            while (nodePages[i]->getPageOffset() != pageOffset) {
                i++;
            }
            nodePages[i]->markDirty();
        }
        
        // Done using this page
        void releasePage(size_t pageOffset) {

        }
        
        // Tree is done with this page
        void freePage(size_t pageOffset) {

        }

        void rootLock(size_t newRootOffset) {

        }

        int getFileDescriptor () {
            return fd;
        }

};
#endif