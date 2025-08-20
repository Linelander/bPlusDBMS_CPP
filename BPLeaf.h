#include "BPNode.h"
#include <algorithm>
#include <any>
#include <cstddef>
#include<iostream>
#include "Item.h"
#include <stdexcept>
#include <vector>
#include <memory>
#include <iterator>
#include <unistd.h>
#include "BPInternalNode.h"
#include "ItemInterface.h"

using namespace std;


// class Item;
template<typename T, int way> class BPInternalNode;

#ifndef BP_LEAF
#define BP_LEAF

template<typename T, int way>
class BPLeaf : public BPNode<T, way> {
    private:
        int itemKeyIndex;
        bool rootBool{false};
        size_t pageSize = 4096;
        vector<ItemInterface*> items; // ItemInterface* or ItemInterface?
        BPNode<T, way>* next = nullptr;
        BPNode<T, way>* prev = nullptr;
        
        // Disk
        size_t pageIndex;
        Freelist* freelist;
        
    public:
        virtual ~BPLeaf() {
            for (int i = 0; i < items.size(); i++)
            {
                delete items[i];
            }
        }
        
        // METHODS
        
        BPLeaf(int keyIndex, Freelist* fList) {
            long foundSize = sysconf(_SC_PAGESIZE);
            pageSize = foundSize;
            this->itemKeyIndex = keyIndex;
            this->freelist = fList;
            pageIndex = freelist->allocate();
        }
        
        BPLeaf(int keyIndex, Freelist* fList, size_t nonstandardSize) {
            this->pageSize = nonstandardSize;
            this->itemKeyIndex = keyIndex;
            this->freelist = fList;
            pageIndex = freelist->allocate();
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
            BPLeaf *newLeaf = new BPLeaf(itemKeyIndex, pageSize);
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
                BPInternalNode<T, way>* newParent = new BPInternalNode<T, way>(itemKeyIndex, pageSize);
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
            cout << "D" << depth << "-L:";
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
                cout << "D" << depth << "-L:";
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
};


#endif