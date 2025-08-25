/*
This is a B+ tree, capable of forming clustered and nonclustered indexes on all columns of Items.

Factories are provided to instantiate trees with branching factors of 3, 5, 8, 16, 100, and 128.
*/


#include "BPNode.h"
#include "ItemInterface.h"

#include <cstddef>
#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>

// Disk
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include "Bufferpool.h"

#define DEFAULT_INDEX_SIZE 64

using namespace std;

#ifndef BP_TREE
#define BP_TREE

template <typename T>
class BPlusTreeBase {
    public:
        virtual vector<uint8_t> getBytes() = 0;
        virtual ~BPlusTreeBase() = default;
        virtual std::array<char, COLUMN_LENGTH> getColumnName() = 0;
        virtual void writeHeader() = 0;
        virtual void insert(ItemInterface* newItem) = 0;
        virtual ItemInterface* remove(T deleteIt) = 0;
        virtual ItemInterface* singleKeySearch(T findIt) = 0;
        virtual void print() = 0;
        virtual void ripPrint() = 0;
        
        // Disk
        virtual void openIndexFile(string name) = 0;
        virtual void rehydrate() = 0;
    };
    
    
    
template <typename T, int way>
class BPlusTree : public BPlusTreeBase<T> {
    private:
        vector<uint8_t> getBytes();
        BPNode<T, way>* root{};
        size_t rootPageOffset;
        int columnCount;
        std::array<char, COLUMN_LENGTH> columnName; // fixed length
        int itemKeyIndex;
        void insert(ItemInterface* newItem);
        size_t pageSize = 4096;
        Bufferpool<T, way>* bufferpool;
    
    public:
        ~BPlusTree();
        std::array<char, COLUMN_LENGTH> getColumnName();
        void writeHeader();
        BPlusTree(int keyIndex, int colCount, string tableName, std::array<char, COLUMN_LENGTH> columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree);
        BPlusTree(int keyIndex, int colCount, string tableName, std::array<char, COLUMN_LENGTH> columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree, size_t nonstandardSize);
        ItemInterface* remove(T deleteIt);
        ItemInterface* singleKeySearch(T findIt);
        void print();
        void ripPrint();
        
        // Disk
        void openIndexFile(string name, std::shared_ptr<BPlusTreeBase<int>> mainTree);
        void rehydrate();
};



template <typename T, int way>
std::array<char, COLUMN_LENGTH> BPlusTree<T, way>::getColumnName() {
    return columnName;
}



template <typename T, int way>
void BPlusTree<T, way>::openIndexFile(string name, std::shared_ptr<BPlusTreeBase<int>> mainTree) {



    // Creates file with naming scheme Tablename_1.bptree
    std::string filename = name + "_" + std::to_string(itemKeyIndex) + ".bptree";
    
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0644);
    ftruncate(fd, pageSize); // start with one page

    if (fd == -1) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), 
        "Failed to open file '%s': %s", filename.c_str(), strerror(errno));
        
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
    
    // Bufferpool(size_t pSize, int file, int colCount, int itemKeyIndex, std::shared_ptr<BPlusTreeBase<int>> mainTree)
    bufferpool = new Bufferpool<T, way>(pageSize, fd, columnCount, itemKeyIndex, mainTree);
}


#include "BPLeaf.h"
template <typename T, int way>
BPlusTree<T, way>::BPlusTree(int keyIndex, int colCount, string tableName, std::array<char, COLUMN_LENGTH> columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree) {    
    
    size_t foundSize = sysconf(_SC_PAGESIZE);
    
    itemKeyIndex = keyIndex;
    columnCount = colCount;
    this->tableName = tableName;
    this->columnName = columnName;
    
    openIndexFile(tableName, mainTree);
    
    // BPLeaf(int keyIndex, int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool) {
    root = new BPLeaf<T, way>(keyIndex, columnCount, mainTree, bufferpool);
    root->makeRoot();
    rootPageOffset = root->getPage()->getPageOffset();
}

// Real class
template <typename T, int way>
BPlusTree<T, way>::BPlusTree(int keyIndex, int colCount, string tableName, std::array<char, COLUMN_LENGTH> columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree, size_t nonstandardSize) {
    itemKeyIndex = keyIndex;
    columnCount = colCount;
    this->tableName = tableName;
    this->columnName = columnName;
    
    openIndexFile(tableName); // also sets bufferpool

    root = new BPLeaf<T, way>(keyIndex, columnCount, mainTree, bufferpool);
    root->makeRoot();
    rootPageOffset = root->getPage()->getPageOffset();

    // Write header
}

template <typename T, int way>
void BPlusTree<T, way>::writeHeader() {
    int fd = bufferpool->getFileDescriptor();
    lseek(fd, 0, SEEK_SET);

    vector<uint8_t> bytes = getBytes();
    int result = write(fd, bytes.data(), bytes.size());

    if (result == -1) {
        cout << strerror(errno);
    }
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



#include "Utils.h"
template <typename T, int way>
vector<uint8_t> BPlusTree<T, way>::getBytes() {
    // We reserve a page for a header
    //TODO: what does this header need to hold?
    /*
        int itemKeyIndex;
        root offset
        way
        column length array columnName;
        freelist
            numbools
            bools (variable length)
    */

    vector<uint8_t> bytes;

    Utils::appendBytes(bytes, itemKeyIndex);                                // 4 bytes
    Utils::appendBytes(bytes, rootPageOffset);                              // size_t bytes
    int wayVal = way;
    Utils::appendBytes(bytes, wayVal);                                      // 4 bytes
    
    for (int i = 0; i < COLUMN_LENGTH; i++) {
        Utils::appendBytes(bytes, static_cast<uint8_t>(columnName[i]));     // COLUMN_LENGTH bytes I think
    }
    
    Utils::appendBytes(bytes, bufferpool->getFreelistBytes());              // VARIABLE bytes - this section starts
                                                                                     // with numbools. Each bool is a byte (not bit packing)
}


// FACTORIES
template<typename T>
std::shared_ptr<BPlusTreeBase<T>> createBPlusTree(int way, int keyIndex, int colCount, string tableName, string columnName) {
    switch(way) {
        case 3: return std::make_unique<BPlusTree<T, 3>>(keyIndex, colCount, tableName, columnName);
        case 5: return std::make_unique<BPlusTree<T, 5>>(keyIndex, colCount, tableName, columnName);
        case 8: return std::make_unique<BPlusTree<T, 8>>(keyIndex, colCount, tableName, columnName);
        case 16: return std::make_unique<BPlusTree<T, 16>>(keyIndex, colCount, tableName, columnName);
        case 100: return std::make_unique<BPlusTree<T, 100>>(keyIndex, colCount, tableName, columnName);
        case 128: return std::make_unique<BPlusTree<T, 128>>(keyIndex, colCount, tableName, columnName);
        default: 
            throw std::invalid_argument("Bad way value: " + std::to_string(way));
    }
}

template<typename T>
std::shared_ptr<BPlusTreeBase<T>> createBPlusTree(int way, int keyIndex, int colCount, string tableName, string columnName, size_t pageSize) {
    switch(way) {
        case 3: return std::make_unique<BPlusTree<T, 3>>(keyIndex, colCount, tableName, columnName, pageSize);
        case 5: return std::make_unique<BPlusTree<T, 5>>(keyIndex, colCount, tableName, columnName, pageSize);
        case 8: return std::make_unique<BPlusTree<T, 8>>(keyIndex, colCount, tableName, columnName, pageSize);
        case 16: return std::make_unique<BPlusTree<T, 16>>(keyIndex, colCount, tableName, columnName, pageSize);
        case 100: return std::make_unique<BPlusTree<T, 100>>(keyIndex, colCount, tableName, columnName, pageSize);
        case 128: return std::make_unique<BPlusTree<T, 128>>(keyIndex, colCount, tableName, columnName, pageSize);
        default: 
            throw std::invalid_argument("Bad way value: " + std::to_string(way));
    }
}

#endif