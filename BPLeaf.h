#include "BPNode.h"
#include <algorithm>
#include <any>
#include <cstddef>
#include<iostream>
#include "Item.h"
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
class BPLeaf : public BPNode<T> {
    private:
        int itemKeyIndex;
        bool rootBool{false};
        size_t pageSize = 4096;
        vector<ItemInterface*> items; // ItemInterface* or ItemInterface?
        BPLeaf<T, way>* neighbor = NULL;
        void setNeighbor(BPLeaf<T, way>* newNeighbor) {neighbor = newNeighbor;}
    
    public:
        virtual ~BPLeaf() {
            for (int i = 0; i < items.size(); i++)
            {
                delete items[i];
            }
        }
        
        // METHODS

        BPLeaf(int keyIndex) {
            long foundSize = sysconf(_SC_PAGESIZE);
            pageSize = foundSize;
            this->itemKeyIndex = keyIndex;
        }

        BPLeaf(int keyIndex, size_t nonstandardSize) {
            this->pageSize = nonstandardSize;
            this->itemKeyIndex = keyIndex;
        }

        // Short Methods
        bool isRoot() {return rootBool;}
        void makeRoot() {rootBool = true;}
        void notRoot() {rootBool = false;}
        bool isLeaf() {return true;}
        int numItems() {return items.size();};

        size_t size() { // TODO: update to account for templating
            
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
        BPNode<T>* split()
        {            
            // Fill the new leaf half way
            BPLeaf *newLeaf = new BPLeaf(itemKeyIndex, pageSize);
            while (newLeaf->numItems() != this->items.size() && newLeaf->numItems() != this->items.size()+1) // new leaf gets half of keys (rounds up for total odd number)
            {
                ItemInterface* pop = items.back();
                items.pop_back();
                
                newLeaf->receiveItem(pop);
            }
            newLeaf->setNeighbor(this->neighbor);
            this->setNeighbor(newLeaf);
            
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



        /*
            IN PROGRESS
        */
        BPNode<T>* insert(ItemInterface* newItem) {
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
                // TODO: duplicate insertion for NCItems. We add the primary key of this duplicate to the PK list.
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


        /*
        This leaf starves and is absorbed by its siblings.

        Siblings must rewire their linked list.
        */
        void merge(BPLeaf<T, way>* leftSibling, BPLeaf<T, way>* rightSibling) {
            if ()
            else if (siblings.size() == 2 && )
        }



        // Removal for poor leaves
        ItemInterface* remove(T deleteIt, BPLeaf<T, way>* leftSibling, BPLeaf<T, way>* rightSibling) {
            
            auto removeLoc = linearSearch(deleteIt);
            ItemInterface* removed = *removeLoc;
            items.erase(removeLoc);
            
            // leaf not wealthy. who do we steal from first?
            if (leftSibling != nullptr && leftSibling->isWealthy()) {
                insert(leftSibling->giveUpLastItem());
                return removed;
            }
            else if (rightSibling != nullptr && rightSibling->isWealthy()) {
                insert(rightSibling.giveUpFirstItem()); // TODO using insert() here is a placeholder - contains a lot of uneccessary checks
                return removed;
            }

            // Neither sibling is wealthy. Merge with left sibling.


        }



        // Removal for wealthy leaves
        ItemInterface* remove(T deleteIt) {
            // For wealthy leaves
            // TODO banal deletion goes here
            ItemInterface* result = *linearSearch(deleteIt);
            // Then we have to let the parent know to change its signposts if we deleted the first one
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
                cout << "EL";
                return;
            }

            for (int i = 0; i < depth; i++)
            {
                cout << "                    ";
            }
            cout << "D" << depth << "-L:";
            printKey(any_cast<T>(items[0]->dynamicGetKeyByIndex(itemKeyIndex)));
            cout << endl;
        }

        int getDepth(int depth) {
            return depth;
        }
};


#endif