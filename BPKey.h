#include <string>

using namespace std;

#ifndef BP_KEY
#define BP_KEY

class BPKey {
    public:  
        string getPrimary();
        string getSecondary();
        string getTertiary();
        string getFullKey();


    private:
        string primary;
        string secondary{};
        string tertiary{};
};

#endif