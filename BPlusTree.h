/*
This is a B+ tree, capable of forming clustered and nonclustered indexes on all columns of Items.

Factories are provided to instantiate trees with branching factors of 3, 5, 8, 16, 100, and 128.
*/

// Bytes: itemKeyIndex(4), rootPageOffset(4), COLUMN_LENGTH(4), numBools(4), variable bools (1)
// 20 + (1 * ?) bytes


#include "BPNode.h"
#include "ItemInterface.h"

#include <cstddef>
#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <filesystem>

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
        virtual void openIndexFile(string name, std::shared_ptr<BPlusTreeBase<int>> mainTree) = 0;
    };
    
    
    
template <typename T, int way>
class BPlusTree : public BPlusTreeBase<T> {
    private:
        vector<uint8_t> getBytes();
        BPNode<T, way>* root{};
        size_t rootPageOffset;
        int columnCount;
        std::array<char, COLUMN_LENGTH> columnName; // fixed length
        string tableName;
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
};



template <typename T, int way>
std::array<char, COLUMN_LENGTH> BPlusTree<T, way>::getColumnName() {
    return columnName;
}



template <typename T, int way>
void BPlusTree<T, way>::openIndexFile(string name, std::shared_ptr<BPlusTreeBase<int>> mainTree) {
    // Creates file with naming scheme Tablename.bptree
    if (name.length() < 7 || name.substr(name.length() - 7) != ".bptree") {
        name = name + ".bptree";
        std::filesystem::path directory = std::filesystem::current_path();
        name = (directory / tableName / name).string();    
    }

    // Open file in read-write mode, creating it if necessary
    int fd = open(name.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), 
        "Failed to open file '%s': %s", name.c_str(), strerror(errno));
        
        // Error handling based on errno
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

    // Check file size to see if it already contains saved freelist data
    struct stat st;
    fstat(fd, &st);
    off_t fileSize = st.st_size;

    bool hasSavedFreelist = false;
    std::vector<uint8_t> savedFreelistBytes;

    int numBools;
    if (fileSize > 0) { // 0 means fresh
        off_t offset = 4 + sizeof(size_t) + COLUMN_LENGTH;
        if (lseek(fd, offset, SEEK_SET) == -1) {
            throw std::runtime_error("Failed to seek to freelist data in the index file.");
        }

        uint8_t buffer[4];
        ssize_t bytesRead = read(fd, buffer, 4);
        if (bytesRead != 4) {
            throw std::runtime_error("Failed to read numBools from index file.");
        }
        memcpy(&numBools, buffer, 4);

        if (numBools > 0) {
            hasSavedFreelist = true;
            savedFreelistBytes.resize(numBools);
            bytesRead = read(fd, savedFreelistBytes.data(), numBools);
            if (bytesRead != numBools) {
                throw std::runtime_error("Failed to read freelist from index file.");
            }
        }
    }

    if (hasSavedFreelist) {
        bufferpool = new Bufferpool<T, way>(pageSize, fd, columnCount, itemKeyIndex, mainTree, savedFreelistBytes, numBools);
    } else {
        bufferpool = new Bufferpool<T, way>(pageSize, fd, columnCount, itemKeyIndex, mainTree);
    }
}







template <typename T, int way>
BPlusTree<T, way>::BPlusTree(int keyIndex, int colCount, string tableName, std::array<char, COLUMN_LENGTH> columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree) {
    size_t foundSize = sysconf(_SC_PAGESIZE);
    itemKeyIndex = keyIndex;
    columnCount = colCount;
    this->tableName = tableName;
    std::copy(columnName.begin(), columnName.end(), this->columnName.begin());

    string colNameStr(columnName.data());
    if (colNameStr.length() >= 7 && colNameStr.substr(colNameStr.length() - 7) == ".bptree") {
        colNameStr = colNameStr.substr(0, colNameStr.length() - 7);
    }
    
    this->columnName.fill('\0');
    size_t copyLength = std::min(colNameStr.length(), static_cast<size_t>(COLUMN_LENGTH - 1));
    std::copy(colNameStr.begin(), 
              colNameStr.begin() + copyLength, 
              this->columnName.begin());
    
    openIndexFile(colNameStr, mainTree);
    
    // Reference: BPLeaf(int keyIndex, int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool)
    root = new BPLeaf<T, way>(keyIndex, columnCount, mainTree, bufferpool); // inits bufferpool
    root->makeRoot();
    rootPageOffset = root->getPage();
    // in theory nobody will ever free the root (as intended)
}




// // Real class
// template <typename T, int way>
// BPlusTree<T, way>::BPlusTree(int keyIndex, int colCount, string tableName, std::array<char, COLUMN_LENGTH> columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree, size_t nonstandardSize) {
//     itemKeyIndex = keyIndex;
//     columnCount = colCount;
//     this->tableName = tableName;
//     std::copy(columnName.begin(), columnName.end(), this->columnName.begin());

//     string colNameStr(columnName.data());
//     if (colNameStr.length() >= 7 && colNameStr.substr(colNameStr.length() - 7) == ".bptree") {
//         colNameStr = colNameStr.substr(0, colNameStr.length() - 7);
//     }
    
//     this->columnName.fill('\0');
//     size_t copyLength = std::min(colNameStr.length(), static_cast<size_t>(COLUMN_LENGTH - 1));
//     std::copy(colNameStr.begin(), 
//               colNameStr.begin() + copyLength, 
//               this->columnName.begin());
    
//     openIndexFile(colNameStr, mainTree); // also sets bufferpool
//     root = new BPLeaf<T, way>(keyIndex, columnCount, mainTree, bufferpool);
//     root->makeRoot();
//     rootPageOffset = root->getPage()->getPageOffset();
// }




template <typename T, int way>
void BPlusTree<T, way>::writeHeader() {
    int fd = bufferpool->getFileDescriptor();
    lseek(fd, 0, SEEK_SET);

    vector<uint8_t> bytes = getBytes();
    checkRW(write(fd, bytes.data(), bytes.size()), fd);
}


template <typename T, int way>
BPlusTree<T, way>::~BPlusTree() {
    delete root;
    delete bufferpool;
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
        root = bufferpool->getNode(root->overthrowRoot());
        root->makeRoot();
        rootPageOffset = root->getPageOffset();
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
        column length array columnName;
        freelist
            numbools
            bools (variable length)
    */

    vector<uint8_t> bytes;

    Utils::appendBytes(bytes, itemKeyIndex);                                // 4 bytes
    Utils::appendBytes(bytes, rootPageOffset);                              // size_t bytes

    
    for (int i = 0; i < COLUMN_LENGTH; i++) {
        Utils::appendBytes(bytes, static_cast<uint8_t>(columnName[i]));     // COLUMN_LENGTH bytes I think
    }
    
    Utils::appendBytes(bytes, bufferpool->getFreelistBytes());              // VARIABLE bytes - this section starts
                                                                                     // with numbools. Each bool is a byte (not bit packing in this version)
}





// FACTORIES
template<typename T>
std::shared_ptr<BPlusTreeBase<T>> createBPlusTree(int way, int keyIndex, int colCount, string tableName, string columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree) {
    switch(way) {
        case 3: return std::make_unique<BPlusTree<T, 3>>(keyIndex, colCount, tableName, columnName, mainTree);
        case 5: return std::make_unique<BPlusTree<T, 5>>(keyIndex, colCount, tableName, columnName, mainTree);
        case 8: return std::make_unique<BPlusTree<T, 8>>(keyIndex, colCount, tableName, columnName, mainTree);
        case 16: return std::make_unique<BPlusTree<T, 16>>(keyIndex, colCount, tableName, columnName, mainTree);
        case 100: return std::make_unique<BPlusTree<T, 100>>(keyIndex, colCount, tableName, columnName, mainTree);
        case 128: return std::make_unique<BPlusTree<T, 128>>(keyIndex, colCount, tableName, columnName, mainTree);
        default: 
            throw std::invalid_argument("Bad way value: " + std::to_string(way));
    }
}


// BPlusTree<T, way>::BPlusTree(int keyIndex, int colCount, string tableName, std::array<char, COLUMN_LENGTH> columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree) {    


// template<typename T>
// std::shared_ptr<BPlusTreeBase<T>> createBPlusTree(int way, int keyIndex, int colCount, string tableName, string columnName, std::shared_ptr<BPlusTreeBase<int>> mainTree, size_t pageSize) {
//     switch(way) {
//         case 3: return std::make_unique<BPlusTree<T, 3>>(keyIndex, colCount, tableName, columnName, mainTree, pageSize);
//         case 5: return std::make_unique<BPlusTree<T, 5>>(keyIndex, colCount, tableName, columnName, mainTree, pageSize);
//         case 8: return std::make_unique<BPlusTree<T, 8>>(keyIndex, colCount, tableName, columnName, mainTree, pageSize);
//         case 16: return std::make_unique<BPlusTree<T, 16>>(keyIndex, colCount, tableName, columnName, mainTree, pageSize);
//         case 100: return std::make_unique<BPlusTree<T, 100>>(keyIndex, colCount, tableName, columnName, mainTree, pageSize);
//         case 128: return std::make_unique<BPlusTree<T, 128>>(keyIndex, colCount, tableName, columnName, mainTree, pageSize);
//         default: 
//             throw std::invalid_argument("Bad way value: " + std::to_string(way));
//     }
// }

#endif