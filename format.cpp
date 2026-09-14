#include <iostream>
#include <string>

using namespace std;

string zeroPad(int number, int size) {
    string str = to_string(number);
    if (str.length() >= size) {
        return str;
    }

    return string(size - str.length(), '0') + str;
}   

int main() {
    cout << zeroPad(42, 5) << '\n';    // 00042
    cout << zeroPad(123, 8) << '\n';   // 00000123
    cout << zeroPad(7, 3) << '\n';     // 007
}