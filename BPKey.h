#include <iostream>
#include <string>
#include <vector>

using namespace std;

#ifndef BP_KEY
#define BP_KEY

class BPKey {
    public:  
        void addKey(string addee);
        void delKey(string del);
        vector<string> getKeys();
        string getFullKey();
        template<typename... Args>
        string pickKeys(Args... keyNums);

    private:
        vector<string> keys;
};

#endif