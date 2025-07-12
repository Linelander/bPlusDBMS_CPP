#include "../BPKey.h"
#include <iostream>
#include <assert.h>

using namespace std;

void testStringKeys()
{
    BPKey key("a");
    key.addKey("b");
    key.addKey("c");
    
    string result = key.getFullKey();
    assert(result == "abc");
    cout << result;
}

int main() {
    testStringKeys();
}