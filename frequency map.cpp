#include <iostream>
#include <string>
using namespace std;
int main() {
    string s;
    cout << "Nhap chuoi ky tu: ";
    getline(cin, s);
    int freq[256] = {0};
    for (int i = 0; i < s.length(); i++) {
    char c = s[i];
    freq[(int)c]++;
    }
    cout << "\n--- Bang tan suat ---\n";
    cout << "Ky tu\tTan suat\n";
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
        char c = (char)i;
        if (c == ' ')
        cout << "(space)\t" << freq[i] << endl;
        else
        cout << c << "\t" << freq[i] << endl;
        }
    }
    int maxFreq = 0;
    char maxChar = ' ';
    for (int i = 0; i < 256; i++) {
        if (freq[i] > maxFreq) {
        maxFreq = freq[i];
        maxChar = (char)i;
        }
    }
    cout << "\nKy tu xuat hien nhieu nhat la '" << maxChar
         << "' voi " << maxFreq << " lan.\n";
    int total = 0, unique = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
        total += freq[i];
        unique++;
        }
    }
    cout << "Tong so ky tu: " << total << endl;
    cout << "So ky tu khac nhau: " << unique << endl;
    return 0;
}
