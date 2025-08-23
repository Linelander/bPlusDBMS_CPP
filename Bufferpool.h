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
#include <stdexcept>
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
        int itemKeyIndex;
        int columnCount;
        std::shared_ptr<BPlusTreeBase<int>> clusteredIndex;
        NodePage<T, way>* root;
        vector<NodePage<T, way>*> nodePages; // More recently used pages go to the end of the vector. Using LRU.
                                                // remember to limit size
        int bufferTargetSize = DEFAULT_BUFFER_TARGET_SIZE;

    public:
        Bufferpool(size_t pSize, int file, int colCount, int itemKeyIndex, std::shared_ptr<BPlusTreeBase<int>> mainTree) : fd(file) {
            freelist = new Freelist(pSize);
            pageSize = pSize;
            clusteredIndex = std::move(mainTree);
            columnCount = colCount;
            this->itemKeyIndex = itemKeyIndex;
        }

        ~Bufferpool() {
            for (NodePage<T, way>* page : nodePages) {
                delete page;
            }
            delete freelist;
            delete root;
        }

        NodePage<T, way>* getPage(size_t pageOffset) {
            NodePage<T, way>* found = nullptr;
            for (int i = 0; i < nodePages.size(); i++)
            {
                if (nodePages[i]->getPageOffset() == pageOffset)
                {
                    found = nodePages[i];
                }
            }
            return found;
        }

        bool isFull() {
            return nodePages.size() >= bufferTargetSize;
        }



        int getPageIndex(size_t pageOffset) {
            for (int i = 0; i < nodePages.size(); i++) {
                if (nodePages[i]->getPageOffset() == pageOffset) {
                    return i;
                }
            }
            return -1;  // Not found
        }



        // Mark page as being used and cycle it to the end of the vector (indicating that it was just used)
        void usePage(size_t pageOffset) {
            int i = getPageIndex(pageOffset);
            if (i == -1) return;
            nodePages[i]->use();
            rotate(nodePages.begin() + i, nodePages.begin() + i + 1, nodePages.end());
        }





        // Tree is done with this page
        void freePage(size_t freed) {
            getPage(freed)->release();
        }





        // Retrieval of an existing node
        BPNode<T, way>* getNode(size_t pageOffset) {
            // Check allocation in freelist
            if (!freelist->isAllocated(pageOffset)) {
                // Issue: nothing with that offset.
                return nullptr;
            }

            // Search the bufferpool
            for (int i = 0; i < nodePages.size(); i++)
            {
                if (nodePages[i]->getPageOffset() == pageOffset)
                {
                    cout << "cache hit" << endl;
                    usePage(pageOffset);
                    return nodePages[i]->getNode(); // Cache hit
                }
            }

            cout << "cache miss" << endl;

            lseek(fd, pageOffset, SEEK_SET);
            
            // Read leafness
            bool isLeaf;
            read(fd, &isLeaf, sizeof(bool));
            
            if (isLeaf)
            {
                // READ REST OF HEADER: sizeof(isLeaf) + sizeof(itemKeyIndex) + sizeof(numItems) + sizeof(rootBool) + sizeof(prev) + sizeof(next);
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
                nodePages.push_back(retrievalPage);
                usePage(pageOffset);
                retrieval->givePage(retrievalPage);

                evict();
                return retrieval;
            }

            // TODO: Read internal stuff and construct an internal

            /*
                ...
            */

        }
        






        // Creation of a new page
        NodePage<T, way>* allocate(BPNode<T, way>* newNode) {
            size_t offset = freelist->allocate();
            NodePage<T, way>* newPage = new NodePage<T, way>(newNode, offset);
            newPage->use();
            evict();
            nodePages.push_back(newPage);
            return newPage;
        }
        




        // For deleting nodes.
        void deallocate(size_t pageOffset) {
            // Always deallocate
            freelist->deallocate(pageOffset);

            // If it's in the pool, remove. No write.
            int i = getPageIndex(pageOffset);
            if (i >= 0)
            {
                delete nodePages[i];
                nodePages.erase(nodePages.begin() + i);
            }
        }


        // Evict the least recently used page (stored near front of vector). Write it if it's dirty.
        void evict()
        {
            if (!isFull()) return;

            // Start looking at the LRU pages. Evict the first one that isn't being used
            for (int i = 0; i < nodePages.size(); i++)
            {
                if (!nodePages[i]->getCurrentlyUsing() && nodePages[i])
                {
                    if (nodePages[i]->getDirty())
                    {
                        nodePages[i].getRAMNode->dehydrate();
                    }
                    delete nodePages[i];
                    nodePages.erase(nodePages.begin() + i);
                    return;
                }
            }

            cout << "No evictable pages" << endl;
        }




        void markDirty(size_t pageOffset) {
            int i = getPageIndex(pageOffset);
            if (i == -1) {
                throw std::runtime_error("Node with given offset not in bufferpool");
            }

            nodePages[i]->markDirty();
        }


        // If new root is in the regular bufferpool, we need to remove it from there
        // New roots can be created from splits or deletions (overthrow)
        void rootLock(size_t rootOffset) {
            // First check if it's already in the buffer pool
            int i = getPageIndex(rootOffset);
            if (i >= 0) {
                // Found in buffer pool - move it to root and remove from pool
                root = nodePages[i];
                nodePages.erase(nodePages.begin() + i);
                return;
            }
            
            // Not in buffer pool - load from disk
            BPNode<T, way>* rootNode = getNode(rootOffset);
            if (rootNode != nullptr) {
                // getNode() added it to the buffer pool, so we need to remove it
                int newIndex = getPageIndex(rootOffset);
                if (newIndex >= 0) {
                    root = nodePages[newIndex];
                    nodePages.erase(nodePages.begin() + newIndex);
                }
            }
        }

        int getFileDescriptor () {
            return fd;
        }



        vector<uint8_t> getFreelistBytes() {
            return freelist->getBytes();
        }

};
#endif