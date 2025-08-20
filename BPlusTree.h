/*
This is a B+ tree, capable of forming clustered and nonclustered indexes on all columns of Items.

Factories are provided to instantiate trees with branching factors of 3, 5, 8, 16, 100, and 128.
*/


#include "BPNode.h"

#include <cstddef>
#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>

// Disk
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

#define DEFAULT_INDEX_SIZE 64

using namespace std;

#ifndef BP_TREE
#define BP_TREE

template <typename T>
class BPlusTreeBase {
    public:
        virtual ~BPlusTreeBase() = default;
        virtual void getColumnName() = 0;
        virtual void openIndexFile(string name) = 0;
        virtual void insert(ItemInterface* newItem) = 0;
        virtual ItemInterface* remove(T deleteIt) = 0;
        virtual ItemInterface* singleKeySearch(T findIt) = 0;
        virtual void print() = 0;
        virtual void ripPrint() = 0;
    };
    
    
    
template <typename T, int way>
class BPlusTree : public BPlusTreeBase<T> {
    private:
        string columnName;
        int itemKeyIndex;
        BPNode<T, way>* root{};
        size_t pageSize = 4096;
        const Freelist* freelist;
    
    public:
        ~BPlusTree();
        string getColumnName();
        void openIndexFile(string name);
        BPlusTree(int keyIndex, string name, string columnName);
        BPlusTree(int keyIndex, string name, string columnName, size_t nonstandardSize);
        void insert(ItemInterface* newItem);
        ItemInterface* remove(T deleteIt);
        ItemInterface* singleKeySearch(T findIt);
        void print();
        void ripPrint();
};



template <typename T, int way>
string BPlusTree<T, way>::getColumnName() {
    return columnName;
}



template <typename T, int way>
void BPlusTree<T, way>::openIndexFile(string name) {

    // Creates file with naming scheme Tablename_1.bptree
    std::string filename = name + "_" + std::to_string(itemKeyIndex) + ".bptree";
    
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0644);
    ftruncate(fd, pageSize); // start with one page

    if (fd == -1) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), 
                "Failed to open file '%s': %s", filename, strerror(errno));
        
        switch (errno) {
            case EACCES:
                throw std::runtime_error("Permission denied: " + std::string(error_msg));
            case ENOENT:
                throw std::runtime_error("File not found: " + std::string(error_msg));
            case ENOSPC:
                throw std::runtime_error("No space left on device: " + std::string(error_msg));
            case EMFILE:
            case ENFILE:
                throw std::runtime_error("Too many open files: " + std::string(error_msg));
            default:
                throw std::runtime_error("File operation failed: " + std::string(error_msg));
        }
    }
}


#include "BPLeaf.h"
template <typename T, int way>
BPlusTree<T, way>::BPlusTree(int keyIndex, string tableName, string columnName) : columnName(columnName) {
    root = new BPLeaf<T, way>(keyIndex);
    root->makeRoot();

    openIndexFile(tableName);

    freelist = new Freelist(pageSize);
}

// Real class
template <typename T, int way>
BPlusTree<T, way>::BPlusTree(int keyIndex, string tableName, string columnName, size_t nonstandardSize) : columnName(columnName) {
    root = new BPLeaf<T, way>(keyIndex, nonstandardSize);
    root->makeRoot();
    
    openIndexFile(tableName);
    
    pageSize = nonstandardSize;
    freelist = new Freelist(pageSize);
}

template <typename T, int way>
BPlusTree<T, way>::~BPlusTree() {
    delete root;
}

template <typename T, int way>
ItemInterface* BPlusTree<T, way>::singleKeySearch(T findIt) {
    return root->singleKeySearch(findIt);
}

template <typename T, int way>
void BPlusTree<T, way>::insert(ItemInterface* newItem) {
    BPNode<T, way>* result = root->insert(newItem);
    if (result != NULL) {
        root = result;
    }
}

template <typename T, int way>
ItemInterface* BPlusTree<T, way>::remove(T deleteIt) {
    ItemInterface* removed = root->remove(deleteIt, nullptr, nullptr).removedItem;
    if (root->getNumChildren() == 1) {
        root = root->overthrowRoot();
    }
    return removed;
}

template <typename T, int way>
void BPlusTree<T, way>::print() {
    root->print(0);
}

template <typename T, int way>
void BPlusTree<T, way>::ripPrint() {
    root->ripPrint(0);
}


// FACTORIES
template<typename T>
std::shared_ptr<BPlusTreeBase<T>> createBPlusTree(int way, int keyIndex, string name) {
    switch(way) {
        case 3: return std::make_unique<BPlusTree<T, 3>>(keyIndex, name);
        case 5: return std::make_unique<BPlusTree<T, 5>>(keyIndex, name);
        case 8: return std::make_unique<BPlusTree<T, 8>>(keyIndex, name);
        case 16: return std::make_unique<BPlusTree<T, 16>>(keyIndex, name);
        case 100: return std::make_unique<BPlusTree<T, 100>>(keyIndex, name);
        case 128: return std::make_unique<BPlusTree<T, 128>>(keyIndex, name);
        default: 
            throw std::invalid_argument("Bad way value: " + std::to_string(way));
    }
}

template<typename T>
std::shared_ptr<BPlusTreeBase<T>> createBPlusTree(int way, int keyIndex, string name, size_t pageSize) {
    switch(way) {
        case 3: return std::make_unique<BPlusTree<T, 3>>(keyIndex, name, pageSize);
        case 5: return std::make_unique<BPlusTree<T, 5>>(keyIndex, name, pageSize);
        case 8: return std::make_unique<BPlusTree<T, 8>>(keyIndex, name, pageSize);
        case 16: return std::make_unique<BPlusTree<T, 16>>(keyIndex, name, pageSize);
        case 100: return std::make_unique<BPlusTree<T, 100>>(keyIndex, name, pageSize);
        case 128: return std::make_unique<BPlusTree<T, 128>>(keyIndex, name, pageSize);
        default: 
            throw std::invalid_argument("Bad way value: " + std::to_string(way));
    }
}

#endif