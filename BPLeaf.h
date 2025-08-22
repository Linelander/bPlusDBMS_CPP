#include "BPNode.h"
#include <algorithm>
#include <any>
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
#include "NCItem.h"

// Disk
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

using namespace std;


// class Item;
template<typename T, int way> class BPInternalNode;

#ifndef BP_LEAF
#define BP_LEAF

template<typename T, int way>
class BPLeaf : public BPNode<T, way> {
    private:
        bool isLeaf = true;
        int itemKeyIndex;
        std::shared_ptr<BPlusTreeBase<int>> clusteredIndex; // in the clustered tree, this will be nullptr.
        int columnCount;
        bool rootBool{false};
        size_t pageSize = 4096;
        vector<ItemInterface*> items; // ItemInterface* or ItemInterface?
        size_t next{}; // need some sort of recognizable default...
        size_t prev{};
        int numItems = 0;
        
        // Disk
        NodePage<T, way>* page;
        Bufferpool<T, way>* bufferpool;

    public:
        virtual ~BPLeaf() {
            for (int i = 0; i < items.size(); i++)
            {
                delete items[i];
            }
        }
        
        // METHODS
        
        BPLeaf(int keyIndex, int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool) {
            size_t foundSize = sysconf(_SC_PAGESIZE);
            pageSize = foundSize;
            this->itemKeyIndex = keyIndex;
            this->bufferpool = bPool;
            // page = bufferpool->allocate(this);
            columnCount = colCount;
            clusteredIndex = std::move(mainTree);
        }
        
        BPLeaf(int keyIndex, int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, size_t nonstandardSize) {
            this->pageSize = nonstandardSize;
            this->itemKeyIndex = keyIndex;
            this->bufferpool = bPool;
            // page = bufferpool->allocate(this); // need to pass this
            columnCount = colCount;
            clusteredIndex = std::move(mainTree);
        }



        // size_t headerSize = sizeof(itemKeyIndex) + sizeof(numItems) + sizeof(rootBool) + sizeof(prev) + sizeof(next);

        // Rehydration constructor
        BPLeaf(int keyIndex, int numItems, bool rootBool, size_t prev, size_t next, int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, size_t pageSize) {
            itemKeyIndex = keyIndex;
            this->numItems = numItems;
            this->rootBool = rootBool;
            this->prev = prev;
            this->next = next;
            this->columnCount = colCount;
            this->clusteredIndex = std::move(mainTree);
            bufferpool = bPool;
        }





        void givePage(NodePage<T, way>* thisPage) {
            page = thisPage;
        }
        
        // Short Methods
        void setNext(BPNode<T, way>* newNext) {next = newNext;}
        void setPrev(BPNode<T, way>* newPrev) {prev = newPrev;}
        BPNode<T, way>* getNext() {return next;}

        bool isRoot() {return rootBool;}
        void makeRoot() {rootBool = true;}
        void notRoot() {rootBool = false;}
        bool isLeaf() {return true;}
        int numItems() {return items.size();}
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
        }

        /*
            This implementation is a "rightward" split
        */
        BPNode<T, way>* split()
        {            
            // Fill the new leaf half way
            BPLeaf *newLeaf = new BPLeaf(itemKeyIndex, bufferpool, pageSize);
            while (newLeaf->numItems() != this->items.size() && newLeaf->numItems() != this->items.size()+1) // new leaf gets half of keys (rounds up for total odd number)
            {
                ItemInterface* pop = items.back();
                items.pop_back();
                
                newLeaf->receiveItem(pop);
            }

            // Rewire
            if (next != nullptr) {
                next->setPrev(newLeaf);
            }
            newLeaf->setPrev(this);
            newLeaf->setNext(this->next);
            this->setNext(newLeaf);


            // If this leaf node is the root, we need to return a new parent of both of these children
            if (isRoot())
            {
                BPInternalNode<T, way>* newParent = new BPInternalNode<T, way>(itemKeyIndex, bufferpool, pageSize);
                newParent->makeRoot();
                
                this->notRoot();

                std::array<BPLeaf*, 2> adopt = {this, newLeaf};
                newParent->becomeFirstInternalRoot(adopt);

                return newParent;
            }

            return newLeaf; // the parent needs to add this to its list of children
        }




        /*
        Linear search for a location in the items vector
        */
        auto linearSearch(T key) {
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




        BPNode<T, way>* insert(ItemInterface* newItem) {
            if (items.size() == 0) {
                items.push_back(newItem);
                return NULL;
            }

            auto itr = linearSearch(any_cast<T>(newItem->dynamicGetKeyByIndex(itemKeyIndex)));
            T newItemKey = any_cast<T>(newItem->dynamicGetKeyByIndex(itemKeyIndex));

            if (itr == items.end())
            {
                items.push_back(newItem);
            }
            else if (any_cast<T>((*itr)->dynamicGetKeyByIndex(itemKeyIndex)) == newItemKey)
            {
                
                if (strcmp(typeid(newItemKey).name(), "int") == 0) // what?
                {
                    cout << "ERROR: INSERTION - Duplicate keys not supported for ints. ints are reserved for unique primary keys." << endl;
                    return NULL;
                }
                (*itr)->addDupeKey(newItem->getPrimaryKey());
            }
            else if (any_cast<T>((*itr)->dynamicGetKeyByIndex(itemKeyIndex)) > newItemKey)
            {
                items.insert(itr, newItem);
            }
            
            if (checkOverflow())
            {
                return split();
            }
            return NULL;
        }

        bool isWealthy()
        {
            int half = (pageSize / items[0]->size()) / 2;
            return (items.size() >= half + 1);
        }

        ItemInterface* giveUpFirstItem() {
            ItemInterface* front = *items.begin();
            items.erase(items.begin());
            return front;
        }

        ItemInterface* giveUpLastItem() {
            ItemInterface* back = items.back();
            items.pop_back();
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
                while (items.size() > 0) {
                    leftSibling->receiveItem(giveUpLastItem());
                }
                unfinishedResult.action = RemovalAction::MERGED_INTO_LEFT;
               
                leftSibling->setNext(next);
                if (next != nullptr)
                {
                    next->setPrev(leftSibling);
                }

                cout << "---- LEFT MERGE leaf ----" << endl;
            }



            else if (rightSibling != nullptr) {
                while (items.size() > 0) {
                    rightSibling->receiveItem(giveUpLastItem());
                }
                unfinishedResult.action = RemovalAction::MERGED_INTO_RIGHT;

                if (prev != nullptr)
                {
                    prev->setNext(rightSibling);
                }
                rightSibling->setPrev(prev);

                cout << "---- RIGHT MERGE leaf ----" << endl;
            }

            bufferpool->deallocate(page->getPageOffset());

            return unfinishedResult;
        }



        // Removal for poor leaves
        RemovalResult<T> remove(T deleteIt, BPNode<T, way>* leftSibling, BPNode<T, way>* rightSibling) {
            
            // Physical removal
            auto removeLoc = linearSearch(deleteIt);
            ItemInterface* removed = *removeLoc;
            if (removed->isClustered()) {
                removed->removeAll();
            }
            delete removed;
            items.erase(removeLoc);

            // Wealthy leaf case
            if (isWealthy()) {
                cout << "---- SIMPLE REMOVE leaf ----" << endl;
                return RemovalResult<T>(removed, RemovalAction::SIMPLE_REMOVAL);
                // Reminder: the parent might still have to change its signposts if the first record of the leaf was deleted
                // (Unless that leaf is the first in the children list)
            }

            RemovalResult<T> result = RemovalResult<T>(removed, RemovalAction::DEFAULT);
            
            // leaf not wealthy. who do we steal from first?
            if (leftSibling != nullptr && leftSibling->isWealthy()) {
                insert(leftSibling->giveUpLastItem());
                result.action = RemovalAction::STOLE_FROM_LEFT;
                cout << "---- LEFT STEAL leaf ----" << endl;
                return result;
            }
            else if (rightSibling != nullptr && rightSibling->isWealthy()) {
                insert(rightSibling->giveUpFirstItem());
                result.action = RemovalAction::STOLE_FROM_RIGHT;
                cout << "---- RIGHT STEAL leaf ----" << endl;
                return result;
            }

            // Neither sibling is wealthy. Merge
            result = merge(leftSibling, rightSibling, result);
            return result;
        }



        ItemInterface* singleKeySearch(T findIt) {
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
            cout << "D" << depth << "-L-" << "@" << pageIndex << ":";
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
                cout << "D" << depth << "-L-" << "@" << pageIndex << ":";
                int i = 0;
                for (ItemInterface* thing : items)
                {
                    printKey(any_cast<T>(items[i]->dynamicGetKeyByIndex(itemKeyIndex)));
                    cout << ",";
                    i++;
                }
            }
            cout << endl;

            if (next != nullptr) {
                next->ripPrint(depth);
                return;
            }
        }



        // POLYMORPHISM OBLIGATIONS
        void mergeLeftHere(BPNode<T, way>* dyingNode) {throw std::runtime_error("Tried to call an internal merging method on a leaf.");}
        void mergeRightHere(BPNode<T, way>* dyingNode) {throw std::runtime_error("Tried to call an internal merging method on a leaf.");}
        BPNode<T, way>* backSteal() {throw std::runtime_error("Tried to call an internal merging method on a leaf.");}
        BPNode<T, way>* frontSteal() {throw std::runtime_error("Tried to call an internal merging method on a leaf.");}
        BPNode<T, way>* overthrowRoot() {
            throw std::runtime_error("Trying to overthrow leaf");
            return nullptr;
        }


        // DISK



        // Deserialize items and add them to the array
        /*
            looks like this:
            - this leaf's data 
                - int numItems (4 bytes)
                - rootBool(1 byte)
                - prev (sizeOf(size_t) bytes)
                - next (sizeOf(size_t) bytes)
        



                Helper method for rehydrate
        */
        void deserializeItems() {
            // 4 + 4 + 1 + ? + ?
            size_t headerSize = sizeof(bool) + sizeof(numItems) + sizeof(rootBool) + sizeof(prev) + sizeof(next);

            // Start reading here:
            size_t itemsOffset = page->getPageOffset() + headerSize;

            int fd = bufferpool->getFileDescriptor();

            lseek(fd, itemsOffset, SEEK_SET);
            
            if (itemKeyIndex == 0) // clustered
            {
                // Assume we know how many attributes there are
                // load 4 + COLUMN_LENGTH*columnCount bytes
                size_t oneItemSize = sizeof(int) + (COLUMN_LENGTH*columnCount);
                size_t itemsSize = oneItemSize * numItems;
                std::vector<uint8_t> itemsBuffer(itemsSize);
                read(fd, itemsBuffer.data(), itemsSize); // Load all items
                
                for (int i = 0; i < numItems; i++)
                {
                    // Cast the first 4 bytes as an int. that's the PK
                    int primaryKey;
                    memcpy(&primaryKey, itemsBuffer.data() + (i * oneItemSize), sizeof(int));
    
                    // CONTINUE: figure out where to get column count and clustered index pointer.

                    // Get attributes
                    vector<AttributeType> attributes{columnCount};
                    size_t attributesSize = columnCount * sizeof(AttributeType);
                    size_t attributesOffset = (i * oneItemSize) + sizeof(primaryKey);

                    memcpy(attributes.data(), 
                        itemsBuffer.data() + attributesOffset, 
                        attributesSize);
    
                    ItemInterface* item = new Item(primaryKey, attributes);
                    items.push_back(item);
                }
            }
            else // NCItems, which are variable length. TODO: might need to change how leaves split in NC item trees
            {
                int jump = 0;
                for (int i = 0; i < numItems; i++) // how to do a variable skip?
                {
                    lseek(fd, itemsOffset + jump, SEEK_SET);
                    
                    int numKeys;
                    read(fd, &numKeys, sizeof(int));
                    
                    // Get item's keys
                    size_t keysSize = sizeof(int) * numKeys;
                    std::vector<int> pointers(numKeys);
                    read(fd, pointers.data(), keysSize);
                    
                    // Create NCItem
                    ItemInterface* ncItem = new NCItem(pointers, clusteredIndex);
                    items.push_back(ncItem);
                    
                    // Update jump for next item
                    jump += sizeof(numKeys) + keysSize;  // numKeys + keys data
                }
            }
            
        }



        void hydrate() {
            // who hydrates this? parents and neighbors? the bufferpool?
            // hydrator rehydrates us
            deserializeItems();
        }




};


#endif