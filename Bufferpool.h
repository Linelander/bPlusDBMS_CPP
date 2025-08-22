/*

LRU Bufferpool. Allocates disk space with a freelist.

*/

#include <cstddef>
#include <iostream>
#include <memory>


#include "Freelist.h"
#include "NodePage.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>


#define DEFAULT_BUFFER_TARGET_SIZE 20;

template<typename T> class BPlusTreeBase;
template<typename T, int way> class BPLeaf;
template<typename T, int way> class BPInternalNode;

using namespace std;

#ifndef BUFFER_POOL
#define BUFFER_POOL
template <typename T, int way>
class Bufferpool {
    private:
        Freelist* freelist;
        int fd;
        int pageSize;
        int columnCount;
        std::shared_ptr<BPlusTreeBase<int>> clusteredIndex;
        NodePage<T, way>* root;
        vector<NodePage<T, way>*> nodePages; // More recently used pages go to the end of the vector. Using LRU.
                                                // remember to limit size
        int bufferTargetSize = DEFAULT_BUFFER_TARGET_SIZE;

    public:
        Bufferpool(size_t pSize, int file, int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree) : fd(file) {
            freelist = new Freelist(pSize);
            pageSize = pSize;
            clusteredIndex = std::move(mainTree);
            columnCount = colCount;
        }

        NodePage<T, way>* getPage(size_t pageOffset) {

        }

        bool isFull() {
            return nodePages.size() >= bufferTargetSize;
        }



        // Retrieval of an existing node
        BPNode<T, way>* getNode(size_t pageOffset) {
            // Search the bufferpool
            for (int i = 0; i < nodePages.size(); i++)
            {
                if (nodePages[i]->getPageOffset() == pageOffset)
                {
                    cout << "cache hit" << endl;            
                    return nodePages[i]->getNode(); // Cache hit
                }
            }

            cout << "cache miss" << endl;
            lseek(fd, pageOffset, SEEK_SET);
            
            // read leafness
            
            bool isLeaf;
            read(fd, &isLeaf, sizeof(bool));
            
            if (isLeaf)
            {
                // READ HEADER: sizeof(isLeaf) + sizeof(itemKeyIndex) + sizeof(numItems) + sizeof(rootBool) + sizeof(prev) + sizeof(next);
                int itemKeyIndex;
                read(fd, &itemKeyIndex, sizeof(int));

                int numItems;
                read(fd, &numItems, sizeof(int));

                bool rootBool;
                read(fd, &rootBool, sizeof(bool));
                
                size_t prev;
                read(fd, &prev, sizeof(size_t));

                size_t next;
                read(fd, &next, sizeof(size_t));

                BPNode<T, way>* retrieval = new BPLeaf(itemKeyIndex, numItems, rootBool, prev, next, columnCount, clusteredIndex, this, pageSize);
                retrieval->deserializeItems();

                NodePage<T, way>* retrievalPage = new NodePage<T, way>(retrieval, pageOffset);
                retrievalPage->use(); // TODO: call this here or have caller do it?
                nodePages.push_back(retrievalPage);

                return retrieval;
            }

            // TODO: Read internal stuff and construct an internal
        }
        

        // Creation of a new page
        NodePage<T, way>* allocate(BPNode<T, way>* newNode) {
            size_t offset = freelist->allocate();
            NodePage<T, way> newPage = new NodePage<T, way>(newNode, offset);
            // Then we put it in the buffer... need to put it in a meaningful spot.
            // Maybe implement pins and sort the buffer based on pins
            
            // TODO continue
        }
        


        int getPageIndex(size_t pageOffset) {
            int i = 0;
            while (nodePages[i]->getPageOffset() != pageOffset) {
                i++;
            }
            return i;
        }


        // Done using this page
        // TODO: add some asserts?
        void deallocate(size_t pageOffset) {
            freelist->deallocate(pageOffset);
            nodePages.erase(nodePages.begin() + getPageIndex(pageOffset));
        }


        // Evict the least recently used page (stored near front of vector). Write it if it's dirty.
        void evict()
        {
            for (int i = 0; i < nodePages.size(); i++)    
            {
                if (!nodePages[i]->getCurrentlyUsing() && nodePages[i])
                {
                    if (nodePages[i]->getDirty())
                    {
                        nodePages[i].getRAMNode.dehydrate();
                    }
                    nodePages.erase(nodePages.begin() + i);
                    break;
                }
            }
        }




        void markDirty(size_t pageOffset) {
            int i = 0;
            while (nodePages[i]->getPageOffset() != pageOffset) {
                i++;
            }
            nodePages[i]->markDirty();
        }
        




        void lockPage(size_t lockee) {
            getPage(lockee)->use();
        }
        



        // Tree is done with this page
        void freePage(size_t freed) {
            getPage(freed)->release();
        }


        // If new root is in the regular bufferpool, we need to remove it from there
        // New roots can be created from splits or deletions (overthrow)
        void rootLock(size_t newRootOffset) {
            root = newRootOffset;
        }

        int getFileDescriptor () {
            return fd;
        }

};
#endif