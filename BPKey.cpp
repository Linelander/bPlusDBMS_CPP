#include <iostream>
#include <string>
#include <list>
#include "BPKey.h"

using namespace std;

// FIELDS
list<string> keys;


// METHODS
void BPKey::addKey(string addee) {
    keys.push_back(addee);
}

void BPKey::delKey(string del) {
    keys.remove(del);
}

list<string> BPKey::getKeys() {
    return this->keys;
}

// Concatenate the specified keys
template<typename... Args>
string pickKeys(Args... keyNums) {
    
}

// Concatenate all keys
string getFullKey() {
    string result{};
}