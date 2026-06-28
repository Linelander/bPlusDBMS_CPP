// HEADER:
// 1     +     4    +    1    +  ?  + ?
// isLeaf, numItems, rootBool, prev, next

#include "BPNode.h"
#include "NCItem.h"
#include <algorithm>
#include <any>
#include <cerrno>
#include <cstddef>
#include <iostream>
#include "Item.h"
#include <stdexcept>
#include <utility>
#include <vector>
#include <memory>
#include <iterator>
#include <unistd.h>
#include "BPInternalNode.h"
#include "ItemInterface.h"
#include "Utils.h"

// Disk
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

using namespace std;


// class Item;
// template<typename T, int way> class BPInternalNode;
template<typename T> class BPlusTreeBase;
// class NCItem;

#ifndef BP_LEAF
#define BP_LEAF

template<typename T, int way>
class BPLeaf : public BPNode<T, way> {
    private:
        int itemKeyIndex;
        std::shared_ptr<BPlusTreeBase<int>> clusteredIndex; // in the clustered tree, this will be nullptr.
        int columnCount;
        bool rootBool{false};
        size_t pageSize = 4096;
        vector<ItemInterface*> items; // ItemInterface* or ItemInterface?
        size_t next = INVALID_PAGE_ID; // need some sort of recognizable default...
        size_t prev = INVALID_PAGE_ID;
        static const size_t INVALID_PAGE_ID = -1;
        
        // Disk
        // NodePage<T, way>* page;
        size_t page;
        Bufferpool<T, way>* bufferpool;
        
    public:
        int numItems = 0;
        bool isLeaf = true;
        bool isLeafFn() {return true;}
        
        // NodePage<T, way> getPage() {return page;};
        

        size_t getPageOffset() {
            // return page->getPageOffset();
            return page;
        }

        virtual ~BPLeaf() {
            for (int i = 0; i < items.size(); i++)
            {
                delete items[i];
            }
        }
        
        vector<uint8_t> getBytes() {
            std::vector<uint8_t> bytes;

            Utils::appendBytes(bytes, isLeaf);        // 1 byte
            Utils::appendBytes(bytes, numItems);
            Utils::appendBytes(bytes, rootBool);      // 1 byte
            Utils::appendBytes(bytes, prev);
            Utils::appendBytes(bytes, next);
           
            for (ItemInterface* item : items)
            {
                Utils::appendBytes(bytes, item->getBytes());
            }

            return bytes;
        }




        // METHODS
        
        BPLeaf(const int keyIndex, const int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool) {
            size_t foundSize = sysconf(_SC_PAGESIZE);
            pageSize = foundSize;
            this->itemKeyIndex = keyIndex;
            this->bufferpool = bPool;
            page = bufferpool->allocate(this);
            columnCount = colCount;
            clusteredIndex = std::move(mainTree);
        }
        
        BPLeaf(const int keyIndex, const int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, const size_t nonstandardSize) {
            this->pageSize = nonstandardSize;
            this->itemKeyIndex = keyIndex;
            this->bufferpool = bPool;
            page = bufferpool->allocate(this);
            columnCount = colCount;
            clusteredIndex = std::move(mainTree);
        }



        // size_t headerSize = sizeof(itemKeyIndex) + sizeof(numItems) + sizeof(rootBool) + sizeof(prev) + sizeof(next);

        // Rehydration constructor
        BPLeaf(const int keyIndex, int numItems, bool rootBool, size_t prev, size_t next, int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, const size_t pageSize, size_t pageOffset) {
            itemKeyIndex = keyIndex;
            this->numItems = numItems;
            this->rootBool = rootBool;
            this->prev = prev;
            this->next = next;
            this->columnCount = colCount;
            page = pageOffset;
            this->clusteredIndex = std::move(mainTree);
            bufferpool = bPool;
        }





        void givePage(size_t offset) {
            page = offset;
        }

        void giveOffset(size_t offset) {
            this->page = offset;
        }
        size_t getPage(){return page;}

        // Short Methods
        void setNext(size_t newNext) {next = newNext;}
        void setPrev(size_t newPrev) {prev = newPrev;}
        size_t getNext() {return next;}

        bool isRoot() {return rootBool;}
        void makeRoot() {rootBool = true;}
        void notRoot() {rootBool = false;}
        // int numItems() {return items.size();}
        int getNumChildren() {return -1;}

        size_t size() {
            size_t leafSize = sizeof(BPLeaf);
            for (ItemInterface* thing : items) {
                leafSize += thing->size();
            }
            return leafSize;
        } // Get the size of this leaf and its items


        bool checkOverflow() {
            size_t currSize = size();
            return (currSize > pageSize);
        } // Is it time to split?


        T viewSign1() {
            auto front = items.begin();
            return any_cast<T>((*front)->dynamicGetKeyByIndex(itemKeyIndex));
        }

        T getSign1()
        {
            auto front = items.begin();
            T result = any_cast<T>((*front)->dynamicGetKeyByIndex(itemKeyIndex));
            return result;
        }

        void receiveItem(ItemInterface* newItem) {
            items.insert(items.begin(), newItem);
            numItems++;
        }

        /*
            This implementation is a "rightward" split
        */
        size_t split()
        {            
            // Fill the new leaf half way
            BPLeaf *newLeaf = new BPLeaf(itemKeyIndex, columnCount, clusteredIndex, bufferpool, pageSize);
            size_t newLeafOffset = newLeaf->getPageOffset();
            while (newLeaf->numItems != this->items.size() && newLeaf->numItems != this->items.size()+1)
            {
                ItemInterface* pop = items.back();
                items.pop_back();
                newLeaf->receiveItem(pop);
            }
            // Sync counter so dehydrate() writes the correct item count
            numItems = (int)items.size();

            // Rewire
            if (bufferpool->getNode(next) != nullptr) {
                bufferpool->getNode(next)->setPrev(newLeaf->getPageOffset());
            }
            newLeaf->setPrev(page);
            newLeaf->setNext(this->next);
            this->setNext(newLeafOffset);


            // If this leaf node is the root, we need to return a new parent of both of these children
            if (isRoot())
            {
                // (const int keyIndex, const int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, size_t nonstandardSize) {
                BPInternalNode<T, way>* newParent = new BPInternalNode<T, way>(itemKeyIndex, columnCount, clusteredIndex, bufferpool, pageSize);
                newParent->makeRoot();
                
                this->notRoot();

                std::array<size_t, 2> adopt = {page, newLeafOffset};
                newParent->becomeFirstInternalRoot(adopt);

                return newParent->getPageOffset();
            }

            return newLeafOffset; // the parent needs to add this to its list of children
        }




        /*
        Linear search for a location in the items vector
        */
        auto linearSearch(const T key) {
            auto curr = items.begin();
            while (true) // BAD. find alternate approach?
            {
                if (curr == items.end())
                {
                    break;
                }
                else if (any_cast<T>((*curr)->dynamicGetKeyByIndex(itemKeyIndex)) == key)
                {
                    break;
                }
                else if (any_cast<T>((*curr)->dynamicGetKeyByIndex(itemKeyIndex)) > key)
                {
                    break;
                }
                curr++;
            }
            return curr;
        }




        size_t insert(ItemInterface* newItem) {
            if (items.size() == 0) {
                items.push_back(newItem);
                numItems++;
                return INVALID_PAGE_ID;
            }

            auto itr = linearSearch(any_cast<T>(newItem->dynamicGetKeyByIndex(itemKeyIndex)));
            T newItemKey = any_cast<T>(newItem->dynamicGetKeyByIndex(itemKeyIndex));

            if (itr == items.end())
            {
                items.push_back(newItem);
                numItems++;
            }
            else if (any_cast<T>((*itr)->dynamicGetKeyByIndex(itemKeyIndex)) == newItemKey)
            {
                
                if constexpr (std::is_same_v<T, int>) {
                    // int == primary key: duplicates not allowed
                    cout << "ERROR: INSERTION - Duplicate primary key " << newItemKey << " rejected." << endl;
                    delete newItem;
                    return INVALID_PAGE_ID;
                }
                (*itr)->addDupeKey(newItem->getPrimaryKey());
            }
            else if (any_cast<T>((*itr)->dynamicGetKeyByIndex(itemKeyIndex)) > newItemKey)
            {
                items.insert(itr, newItem);
                numItems++;
            }
            
            bufferpool->markDirty(page);

            if (checkOverflow())
            {
                return split();
            }
            return INVALID_PAGE_ID;
        }




        bool isWealthy()
        {
            if (items.empty()) return false;
            int half = (int)(pageSize / items[0]->size()) / 2;
            return ((int)items.size() >= half + 1);
        }




        ItemInterface* giveUpFirstItem() {
            ItemInterface* front = *items.begin();
            items.erase(items.begin());
            numItems--;
            return front;
        }




        ItemInterface* giveUpLastItem() {
            ItemInterface* back = items.back();
            items.pop_back();
            numItems--;
            return back;
        }




        T getHardLeft() {
            return any_cast<T>(items[0]->dynamicGetKeyByIndex(itemKeyIndex));
        }




        /*
        This leaf starves and is absorbed by one of its siblings.

        Siblings must rewire their linked list.

        This method decides which sibling to merge with and performs the merge. We favor the left sibling.
        */
        RemovalResult<T> merge(BPNode<T, way>* leftSibling, BPNode<T, way>* rightSibling, RemovalResult<T> unfinishedResult) {
            if (leftSibling != nullptr) {
                // Dying leaf has LARGER keys — append in sorted order to left sibling's back.
                // (receiveItem inserts at the front, which would invert the order here.)
                BPLeaf<T, way>* leftLeaf = static_cast<BPLeaf<T, way>*>(leftSibling);
                while (!items.empty()) {
                    leftLeaf->items.push_back(items.front());
                    leftLeaf->numItems++;
                    items.erase(items.begin());
                    numItems--;
                }
                unfinishedResult.action = RemovalAction::MERGED_INTO_LEFT;

                leftSibling->setNext(next);
                if (bufferpool->getNode(next) != nullptr) {
                    bufferpool->getNode(next)->setPrev(leftSibling->getPageOffset());
                }
            }
            else if (rightSibling != nullptr) {
                // Dying leaf has SMALLER keys — prepend in sorted order to right sibling's front.
                // giveUpLastItem (largest first) + receiveItem (insert at front) reverses correctly.
                while (items.size() > 0) {
                    rightSibling->receiveItem(giveUpLastItem());
                }
                unfinishedResult.action = RemovalAction::MERGED_INTO_RIGHT;

                if (bufferpool->getNode(prev) != nullptr) {
                    bufferpool->getNode(prev)->setNext(rightSibling->getPageOffset());
                }
                rightSibling->setPrev(prev);
            }

            bufferpool->deallocate(page);

            return unfinishedResult;
        }



        // Removal for poor leaves
        RemovalResult<T> remove(const T deleteIt, BPNode<T, way>* leftSibling, BPNode<T, way>* rightSibling) {
            
            // Physical removal
            auto removeLoc = linearSearch(deleteIt);
            ItemInterface* removed = *removeLoc;
            if (!removed->isClustered()) {
                // NCItem: cascade-remove all pointer targets from the clustered index
                removed->removeAll();
            }
            items.erase(removeLoc);
            numItems--;
            // Ownership of `removed` transfers to the caller via RemovalResult.

            // Wealthy leaf case
            if (isWealthy()) {
                return RemovalResult<T>(removed, RemovalAction::SIMPLE_REMOVAL);
                // Reminder: the parent might still have to change its signposts if the first record of the leaf was deleted
                // (Unless that leaf is the first in the children list)
            }

            // Root can be underfull — no merge partner anyway.
            if (leftSibling == nullptr && rightSibling == nullptr) {
                return RemovalResult<T>(removed, RemovalAction::SIMPLE_REMOVAL);
            }

            RemovalResult<T> result = RemovalResult<T>(removed, RemovalAction::DEFAULT);

            // leaf not wealthy. who do we steal from first?
            if (leftSibling != nullptr && leftSibling->isWealthy()) {
                insert(leftSibling->giveUpLastItem());
                result.action = RemovalAction::STOLE_FROM_LEFT;
                return result;
            }
            else if (rightSibling != nullptr && rightSibling->isWealthy()) {
                insert(rightSibling->giveUpFirstItem());
                result.action = RemovalAction::STOLE_FROM_RIGHT;
                return result;
            }

            // Neither sibling is wealthy. Merge.
            // merge() calls deallocate(page), so the page is gone — do NOT markDirty.
            result = merge(leftSibling, rightSibling, result);
            return result;
        }




        ItemInterface* singleKeySearch(const T findIt) {
            auto itemItr = linearSearch(findIt);

            if (itemItr == items.end()) {
                cout << "SEARCH FAILURE: Record with key ";
                printKey(findIt);
                cout << " not found." << endl;
                return {};
            }

            T itemItrKey = any_cast<T>((*itemItr)->dynamicGetKeyByIndex(itemKeyIndex));

            if (itemItrKey == findIt)
            {
                return (*itemItr);
                // return (*itemItr)->singleKeySearchResult();
            }
            cout << "SEARCH FAILURE: Record with key ";
            printKey(findIt);
            cout << " not found." << endl;
            return {};
        }




        void printKey(int key) {
            cout << key;
        }




        void printKey(const AttributeType& attr) {
            cout << attr.data();
        }




        void print(int depth) {
            // Print this:
            if (items.size() == 0)
            {
                cout << "EL" << endl;
                return;
            }

            for (int i = 0; i < depth; i++)
            {
                cout << "                    ";
            }
            cout << "D" << depth << "-L-" << "@" << page << ":";
            int i = 0;
            for (ItemInterface* thing : items)
            {
                printKey(any_cast<T>(items[i]->dynamicGetKeyByIndex(itemKeyIndex)));
                cout << ",";
                i++;
            }
            cout << endl;
        }




        void ripPrint(int depth) {
            
            for (int i = 0; i < depth; i++)
            {
                cout << "                    ";
            }
            if (items.size() == 0)
            {
                cout << "EL";
            }
            else {
                cout << "D" << depth << "-L-" << "@" << page << ":";
                int i = 0;
                for (ItemInterface* thing : items)
                {
                    printKey(any_cast<T>(items[i]->dynamicGetKeyByIndex(itemKeyIndex)));
                    cout << ",";
                    i++;
                }
            }
            cout << endl;

            if (bufferpool->getNode(next) != nullptr) {
                bufferpool->getNode(next)->ripPrint(depth);
                return;
            }
        }




        // POLYMORPHISM OBLIGATIONS
        void mergeLeftHere(BPNode<T, way>* dyingNode) {throw std::runtime_error("Tried to call an internal merging method on a leaf.");}
        void mergeRightHere(BPNode<T, way>* dyingNode) {throw std::runtime_error("Tried to call an internal merging method on a leaf.");}
        size_t backSteal() {throw std::runtime_error("Tried to call an internal merging method on a leaf.");}
        size_t frontSteal() {throw std::runtime_error("Tried to call an internal merging method on a leaf.");}
        size_t overthrowRoot() {
            throw std::runtime_error("Trying to overthrow leaf");
            return INVALID_PAGE_ID;
        }




        //                  DISK


        // Deserialize items and add them to the array
        /*
            looks like this:
            - this leaf's data 
                - int numItems (4 bytes)
                - rootBool(1 byte)
                - prev (sizeOf(size_t) bytes)
                - next (sizeOf(size_t) bytes)

                Helper method for rehydration
        */
        void deserializeItems() {
            // 1     +     4    +    1    +  ?  + ?
            // isLeaf, numItems, rootBool, prev, next
            size_t headerSize = sizeof(isLeaf) + sizeof(numItems) + sizeof(rootBool) + sizeof(prev) + sizeof(next);
            size_t itemsOffset = page + headerSize;

            int fd = bufferpool->getFileDescriptor();

            size_t estimatedItemSize = 0;
            if (itemKeyIndex == 0) { // clustered
                // int (primaryKey) + columnCount * COLUMN_LENGTH (bytes per attribute)
                estimatedItemSize = numItems * (sizeof(int) + columnCount * COLUMN_LENGTH);
            }
            else { // NC items (hard-coded worst case)
                estimatedItemSize = pageSize / 2;
            }

            std::vector<uint8_t> buffer(estimatedItemSize);
            lseek(fd, itemsOffset, SEEK_SET);
            checkRW(read(fd, buffer.data(), buffer.size()), fd); // big read

            size_t offset = 0;
            if (itemKeyIndex == 0) // clustered
            {
                for (int i = 0; i < numItems; i++) {
                    // Read primary key
                    int primaryKey = *reinterpret_cast<int*>(&buffer[offset]);
                    offset += sizeof(int);

                    // Read attributes
                    std::vector<AttributeType> attrs;
                    for (int j = 0; j < columnCount; j++) {
                        AttributeType att;
                        std::memcpy(att.data(), &buffer[offset], COLUMN_LENGTH);
                        offset += COLUMN_LENGTH;
                        attrs.push_back(att);
                    }

                    ItemInterface* item = new Item(primaryKey, attrs);
                    items.push_back(item);
                }
            }
            else // non-clustered (variable-length keys)
            {
                size_t jump = 0; // jump through buffer
                for (int i = 0; i < numItems; i++)
                {
                    int numKeys = *reinterpret_cast<int*>(&buffer[jump]);
                    jump += sizeof(int);

                    std::vector<int> pointers(numKeys);
                    std::memcpy(pointers.data(), &buffer[jump], numKeys * sizeof(int));
                    jump += numKeys * sizeof(int);

                    ItemInterface* ncItem = new NCItem(pointers, clusteredIndex);
                    items.push_back(ncItem);
                }
            }
        }



        void deserializeItemsFromBuffer(const std::vector<uint8_t>& buffer, size_t itemsOffset) {
            size_t offset = itemsOffset;

            if (itemKeyIndex == 0) {
                for (int i = 0; i < numItems; i++) {
                    int primaryKey = *reinterpret_cast<const int*>(&buffer[offset]);
                    offset += sizeof(int);

                    std::vector<AttributeType> attrs;
                    for (int j = 0; j < columnCount; j++) {
                        AttributeType att;
                        std::memcpy(att.data(), &buffer[offset], COLUMN_LENGTH);
                        offset += COLUMN_LENGTH;
                        attrs.push_back(att);
                    }

                    items.push_back(new Item(primaryKey, attrs));
                }
            } 
            else {
                for (int i = 0; i < numItems; i++) {
                    int numKeys = *reinterpret_cast<const int*>(&buffer[offset]);
                    offset += sizeof(int);

                    std::vector<int> pointers(numKeys);
                    std::memcpy(pointers.data(), &buffer[offset], numKeys * sizeof(int));
                    offset += numKeys * sizeof(int);

                    items.push_back(new NCItem(pointers, clusteredIndex));
                }
            }
        }





        void dehydrate() {
            int fd = bufferpool->getFileDescriptor();
            size_t offset = page;
            vector<uint8_t> bytes;

            int actualCount = (int)items.size();  // authoritative — numItems can lag
            appendBytes(bytes, isLeaf);
            appendBytes(bytes, actualCount);
            appendBytes(bytes, rootBool);
            appendBytes(bytes, prev);
            appendBytes(bytes, next);

            for (int i = 0; i < actualCount; i++) {
                vector<uint8_t> itemBytes = items[i]->getBytes();
                bytes.insert(bytes.end(), itemBytes.begin(), itemBytes.end());
            }

            lseek(fd, offset, SEEK_SET);
            checkRW(write(fd, bytes.data(), bytes.size()), fd);
        }




};



#endif