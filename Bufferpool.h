/*

Bufferpool. Maintains a list of pointers to all nodes that get created. A reference to this is in turn held by the nodes themselves.

The bufferpool holds a freelist that it can use to

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
        vector<NodePage<T, way>*> nodePages;

    public:
        Bufferpool(size_t pageSize, int file) : fd(file) {
            freelist = new Freelist(pageSize);
        }



        NodePage<T, way>* getPage(size_t pageOffset) {

        }
        
        NodePage<T, way>* allocate() {

        }
        
        void markDirty(size_t pageOffset);
        
        // Eviction
        void releasePage(size_t pageOffset);        // Done using this page
        
        // Tree is done with this page
        void freePage(size_t pageOffset);

};
#endif