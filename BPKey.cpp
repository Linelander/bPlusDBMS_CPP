#include <iostream>
#include <string>
#include <vector>
#include "BPKey.h"
#include <sstream>
#include <algorithm>

using namespace std;

// FIELDS
vector<string> keys;

// METHODS
BPKey::BPKey()
{
    
}

BPKey::BPKey(string initKey)
{
    addKey(initKey);
}

void BPKey::addKey(string addee) {
    keys.push_back(addee);
}

// NOTE: THIS IMPLEMENTATION MEANS I CAN'T HAVE DUPLICATE KEYS. PLACEHOLDER.
void BPKey::delKey(string del) {
    remove(keys.begin(), keys.end(), del);
  	// Deleting the last element i.e. 7
    keys.pop_back();
}

vector<string> BPKey::getKeys() {
    return this->keys;
}

// Concatenate the specified keys
template<typename... Args>
string BPKey::pickKeys(Args... keyNums) {
    stringstream result;
    for (const auto num : {keyNums...})
    {
        result << keys[num];
    }
    return result.str();
}

// Concatenate all keys
string BPKey::getFullKey() {
    stringstream result;
    for (const auto& elem : keys)
    {
        result << elem;
    }
    return result.str();
}