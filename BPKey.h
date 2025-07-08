#include <iostream>
#include <string>

using namespace std;

#ifndef BP_KEY
#define BP_KEY

class BPKey {
    public:  
        void setPrimary(string set);
        void setSecondary(string set);
        void setTertiary(string set);    

        string getPrimary();
        string getSecondary();
        string getTertiary();
        string getFullKey();


    private:
        string primary{};
        string secondary{};
        string tertiary{};
};

#endif