#include <cstddef>
#include <iostream>
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
        Freelist(int pSize) : pageSize(pSize) {
            bitmap.push_back(FREE);
        }

        size_t allocate() {

        }
};
#endif