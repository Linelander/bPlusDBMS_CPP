#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

#define ALLOCATED true
#define FREE false

#ifndef FREELIST
#define FREELIST
class Freelist {
    private:
        size_t pageSize;
        vector<bool> bitmap;

    public:
        Freelist(size_t pSize) : pageSize(pSize) {
            if (pSize == 0) throw std::invalid_argument("Page size must be > 0");
            bitmap.push_back(FREE);
        }

        // allocate the first free spot and return the page offset
        size_t allocate() {
            size_t i = 0;
            while (i < bitmap.size() && bitmap[i] != FREE) {
                i++;
            }

            // Need to expand the freelist
            if (i == bitmap.size()) {
                bitmap.resize(bitmap.size() * 2, FREE);
            }

            // Found something mid-list
            bitmap[i] = ALLOCATED;
            return i * pageSize;
        }

        void deallocate(size_t offset) {
            if (offset % pageSize != 0) {
                throw std::runtime_error("Given offset doesn't line up with page divisions");
            }
            size_t pageIndex = offset / pageSize;
            if (pageIndex >= bitmap.size()) {
                throw std::runtime_error("Offset out of bounds");
            }
            if (bitmap[pageIndex] == FREE) {
                throw std::runtime_error("Page already deallocated");
            }

            bitmap[pageIndex] = FREE;
        }
};
#endif