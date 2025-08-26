/*

LRU Bufferpool. Allocates disk space with a freelist.

*/

#include <cstddef>
#include <iostream>
#include <memory>


#include "Freelist.h"
#include "NodePage.h"
#include "BPNode.h"
#include "Utils.h"

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
using namespace Utils;

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
        size_t currentFileSize;
        static constexpr size_t GROWTH_CHUNK_PAGES = 10;
        std::shared_ptr<BPlusTreeBase<int>> clusteredIndex;
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
            
            struct stat st;
            fstat(fd, &st);
            currentFileSize = st.st_size;
        }




        ~Bufferpool() {
            for (NodePage<T, way>* page : nodePages) {
                delete page;
            }
            delete freelist;
        }




        NodePage<T, way>* getPage(const size_t pageOffset) {
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
        void usePage(const size_t pageOffset) {
            int i = getPageIndex(pageOffset);
            if (i == -1) return;
            nodePages[i]->use();
            rotate(nodePages.begin() + i, nodePages.begin() + i + 1, nodePages.end());
        }




        // Tree is done with this page
        void freePage(const size_t freed) {
            getPage(freed)->release();
        }




        // Retrieval of an existing node
        BPNode<T, way>* getNode(const size_t pageOffset) {
            if (!freelist->isAllocated(pageOffset)) {
                return nullptr;
            }

            for (int i = 0; i < nodePages.size(); i++) {
                if (nodePages[i]->getPageOffset() == pageOffset) {
                    cout << "!! - CACHE HIT - !!" << endl;
                    usePage(pageOffset);
                    return getNode(nodePages[i]->getPageOffset());
                }
            }

            cout << "// - CACHE MISS - //" << endl;

            const size_t maxNodeSize = pageSize;
            std::vector<uint8_t> buffer(maxNodeSize);

            lseek(fd, pageOffset, SEEK_SET);
            Utils::checkRW(read(fd, buffer.data(), maxNodeSize), fd);

            size_t offset = 0;

            // read leafness
            bool isLeaf = *reinterpret_cast<bool*>(&buffer[offset]);
            offset += sizeof(bool);

            if (isLeaf) { // Leaf
                int numItems = *reinterpret_cast<int*>(&buffer[offset]);
                offset += sizeof(int);

                bool rootBool = *reinterpret_cast<bool*>(&buffer[offset]);
                offset += sizeof(bool);

                size_t prev = *reinterpret_cast<size_t*>(&buffer[offset]);
                offset += sizeof(size_t);

                size_t next = *reinterpret_cast<size_t*>(&buffer[offset]);
                offset += sizeof(size_t);

                evict();

                BPNode<T, way>* retrieval = new BPLeaf<T, way>(
                    itemKeyIndex, numItems, rootBool, prev, next,
                    columnCount, clusteredIndex, this, pageSize, pageOffset
                );

                static_cast<BPLeaf<T, way>*>(retrieval)->deserializeItemsFromBuffer(buffer, offset);

                NodePage<T, way>* retrievalPage = new NodePage<T, way>(retrieval, pageOffset);
                nodePages.push_back(retrievalPage);
                usePage(pageOffset);

                return retrieval;
            } 
            else { // Internal
                bool rootBool = *reinterpret_cast<bool*>(&buffer[offset]);
                offset += sizeof(bool);

                int numSignposts = *reinterpret_cast<int*>(&buffer[offset]);
                offset += sizeof(int);

                int numChildren = *reinterpret_cast<int*>(&buffer[offset]);
                offset += sizeof(int);

                std::vector<T> signposts(numSignposts);
                std::memcpy(signposts.data(), &buffer[offset], numSignposts * sizeof(T));
                offset += numSignposts * sizeof(T);

                std::vector<size_t> children(numChildren);
                std::memcpy(children.data(), &buffer[offset], numChildren * sizeof(size_t));
                offset += numChildren * sizeof(size_t);

                evict();

                BPNode<T, way>* retrieval = new BPInternalNode<T, way>(
                    itemKeyIndex, columnCount, clusteredIndex, this,
                    signposts, children, pageSize
                );

                NodePage<T, way>* retrievalPage = new NodePage<T, way>(retrieval, pageOffset);
                nodePages.push_back(retrievalPage);
                usePage(pageOffset);

                return retrieval;
            }
        }
        



        // Creation of a new page
        size_t allocate(BPNode<T, way>* newNode) {
            size_t offset = freelist->allocate();
            
            size_t requiredSize = offset + pageSize;
            
            if (requiredSize > currentFileSize) {
                // Grow file in chunks to reduce system calls
                size_t newSize = ((requiredSize / pageSize) + GROWTH_CHUNK_PAGES) * pageSize;
                
                if (ftruncate(fd, newSize) == -1) {
                    throw std::runtime_error("Failed to grow file: " + std::string(strerror(errno)));
                }
                
                currentFileSize = newSize;
            }
            
            NodePage<T, way>* newPage = new NodePage<T, way>(newNode, offset);
            newPage->use();
            evict();
            nodePages.push_back(newPage);
            markDirty(offset);
            return offset;
        }
        



        // For deleting nodes.
        void deallocate(const size_t pageOffset) {
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
                        nodePages[i]->getRAMNode()->dehydrate();
                    }
                    delete nodePages[i];
                    nodePages.erase(nodePages.begin() + i);
                    return;
                }
            }

            cout << "No evictable pages" << endl;
        }




        void markDirty(const size_t pageOffset) {
            int i = getPageIndex(pageOffset);
            if (i == -1) {
                throw std::runtime_error("Node with given offset not in bufferpool");
            }

            nodePages[i]->markDirty();
        }




        int getFileDescriptor () {
            return fd;
        }



        vector<uint8_t> getFreelistBytes() {
            return freelist->getBytes();
        }

        // Force writing of all dirty pages. For testing purposes.
        // Acts like evict without the quota checks.
        void writePages() {

            for (int i = 0; i < nodePages.size(); i++)
            {
                if (!nodePages[i]->getCurrentlyUsing() && nodePages[i])
                {
                    if (nodePages[i]->getDirty())
                    {
                        nodePages[i]->getRAMNode()->dehydrate();
                    }
                    delete nodePages[i];
                    nodePages.erase(nodePages.begin() + i);
                }
            }
        }
};
#endif