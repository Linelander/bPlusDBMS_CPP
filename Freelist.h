#include "Utils.h"
#include <cstddef>
#include <cstdint>
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

        static constexpr size_t RESERVED_PAGES = 1; // this is the page that .bptree files store metadata in

    public:
        
    
    Freelist(size_t pSize) : pageSize(pSize) {
            if (pSize == 0) throw std::invalid_argument("Page size must be > 0");
            bitmap.push_back(FREE); // TODO: why are we doing this?
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
            return (i + RESERVED_PAGES) * pageSize;        
        }


        void deallocate(size_t offset) {
            if (offset < RESERVED_PAGES * pageSize || offset % pageSize != 0) {
                throw std::runtime_error("Invalid offset: must be page-aligned and after reserved space");
            }
            
            size_t pageIndex = (offset / pageSize) - RESERVED_PAGES;
            
            if (pageIndex >= bitmap.size()) {
                throw std::runtime_error("Offset out of bounds");
            }
            
            if (bitmap[pageIndex] == FREE) {
                throw std::runtime_error("Page already deallocated");
            }
            
            bitmap[pageIndex] = FREE;
        }


        bool isAllocated(size_t offset) const {
            if ((offset - RESERVED_PAGES) % pageSize != 0) {
                return false; // Invalid offset
            }
            size_t pageIndex = (offset / pageSize) - RESERVED_PAGES;
            if (pageIndex >= bitmap.size()) {
                return false; // Out of bounds
            }
            return bitmap[pageIndex] == ALLOCATED;
        }

        /*
            Return header of no. of bools + bools of freelist
            4 + (1 * numBools) bytes
        */
        vector<uint8_t> getBytes() {
            std::vector<uint8_t> bytes;
            
            // 4-byte header
            // num things
            int numBools = bitmap.size();
            Utils::appendBytes(bytes, numBools);

            for (int i = 0; i < numBools; i++) {
                bytes.push_back(bitmap[i]);
            }

            return bytes;
        }
};
#endif