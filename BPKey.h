#include <iostream>
#include <string>
#include <list>

using namespace std;

#ifndef BP_KEY
#define BP_KEY

class BPKey {
    public:  
        void addKey(string addee);
        void delKey(string del);
        list<string> getKeys();
        string getFullKey();

    private:
        list<string> keys;
};

#endif