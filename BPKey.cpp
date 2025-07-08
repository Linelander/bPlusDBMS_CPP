#include <iostream>
#include <string>

using namespace std;

string primary{};
string secondary{};
string tertiary{};



void setPrimary(string set) {
    primary = set;
}
void setSecondary(string set) {
    secondary = set;
}
void setTertiary(string set) {
    tertiary = set;
}



string getPrimary() {
    return primary;
}
string getSecondary() {
    return secondary;
}
string getTertiary() {
    return tertiary;
}
string getFullKey() {
    string result = primary + secondary + tertiary; // does this actually concatenate?
    return result;
}