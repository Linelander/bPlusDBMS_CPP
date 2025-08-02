#include "BPNode.h"
#include <any>
#include <cstddef>
#include<iostream>
#include "Item.h"
#include <vector>
#include <memory>
#include <iterator>
#include <unistd.h>
#include "BPInternalNode.h"

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
        // BPLeaf<T>* overflow = NULL;
        BPLeaf<T, way>* neighbor = NULL;
    
    public:
        virtual ~BPLeaf() = default;
        
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
        BPLeaf<T, way>* getNeighbor() {return neighbor;}
        vector<ItemInterface*>* accessItems() {return &items;}
        int numItems() {return items.size();};
        void setNeighbor(BPLeaf<T, way>* newNeighbor) {neighbor = newNeighbor;}

        size_t size() { // TODO: update to account for templating
            
            size_t leafSize = sizeof(BPLeaf);
            for (ItemInterface* thing : items) {
                leafSize += sizeof(*thing);
            }
            return leafSize;
        } // Get the size of this leaf and its items


        bool checkOverflow() {
            size_t currSize = size();
            return (currSize > pageSize);
        } // Is it time to split?


        // T viewSign1() {
        //     return (any_cast<T>((*items.begin())->dynamicGetKeyByIndex(itemKeyIndex)));
        // }

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


        /*
            This implementation is a "rightward" split
        */
        BPNode<T>* split()
        {
            
            // TODO: get rid of accessItems()
            // Fill the new leaf half way
            int i = 0;
            BPLeaf *newLeaf = new BPLeaf(itemKeyIndex, pageSize);
            while (newLeaf->numItems() != this->items.size() && newLeaf->numItems() != this->items.size()+1) // new leaf gets half of keys (rounds up for total odd number)
            {
                ItemInterface* pop = items.back();
                items.pop_back();
                
                auto newFront = newLeaf->accessItems()->begin();
                *newLeaf->accessItems()->insert(newFront, pop);
                i++;
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


        int remove(int deleteIt) {
            return -99;
        }


        vector<ItemInterface*> singleKeySearch(T findIt) {
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
                return (*itemItr)->singleKeySearchResult();
            }
            cout << "SEARCH FAILURE: Record with key ";
            printKey(findIt);
            cout << " not found." << endl;
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