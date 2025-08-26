#include <cstddef>
// #include "../BPlusTree.h"
#include "../BPLeaf.h"

using namespace std;

constexpr size_t INVALID_OFFSET = -1;




int makeFile() {
    int fd = open("test.bptree", O_RDWR | O_CREAT, 0644);
    ftruncate(fd, 4096); // start with one page

    if (fd == -1) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), 
        "Failed to open file '%s': %s", "test.bptree", strerror(errno));
        
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

    return fd;
}




int testItems() {
    ItemInterface* item = new Item(0, {{"Hello"}, {"Testing"}, {"1, 2, 3."}});

    
    // Bufferpool(size_t pSize, int file, int colCount, int itemKeyIndex, std::shared_ptr<BPlusTreeBase<int>> mainTree) : fd(file)
    int fd = makeFile();
    Bufferpool<int, 4>* pool = new Bufferpool<int, 4>(4096, fd, 3, 0, nullptr);
    
    
    // BPLeaf(int keyIndex, int colCount, std::shared_ptr<BPlusTreeBase<int>> mainTree, Bufferpool<T, way>* bPool, size_t nonstandardSize) {
    BPLeaf<int, 4>* leaf = new BPLeaf<int, 4>(0, 3, nullptr, pool, 4096);

    std::size_t offset = leaf->getPageOffset();

    leaf->insert(item);

    cout << "Print new RAM leaf:" << endl << endl;
    leaf->print(0);

    cout << endl << "Write leaf to disk and delete from RAM." << endl << endl;

    cout << "Ask bufferpool for leaf" << endl << endl;

    pool->freePage(4096);

    pool->writePages();

    BPNode<int, 4>* retrieval = pool->getNode(offset);
    cout << endl;

    retrieval->print(0);

    cout << endl;


    int result = std::remove("test.bptree");

    if (result == 0) {
        std::cout << "File deleted successfully.\n";
    } else {
        std::perror("Error deleting file");
    }

    return 0;
}




int main() {
    int result = 0;
    result = testItems();
    return result;
}