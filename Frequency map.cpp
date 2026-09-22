#include <iostream>
#include <string>
using namespace std;

int main() {
    string s;
    cout << "Nhap chuoi ky tu: ";
    getline(cin, s);

    // Mảng đếm tần suất xuất hiện của từng ký tự (chỉ số 0-255 tương ứng mã ASCII)
    int freq[256] = {0};
    for (int i = 0; i < s.length(); i++) {
        char c = s[i];
        freq[(int)c]++;   // Ép ký tự thành mã ASCII để làm chỉ số mảng, rồi tăng đếm
    }

    cout << "\n--- Bang tan suat ---\n";
    cout << "Ky tu\tTan suat\n";
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            char c = (char)i;
            if (c == ' ')
                cout << "(space)\t" << freq[i] << endl;  // In riêng ký tự space vì in trực tiếp sẽ khó nhìn
            else
                cout << c << "\t" << freq[i] << endl;
        }
    }

    // Tìm ký tự có tần suất xuất hiện cao nhất
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

    // Tính tổng số ký tự và số ký tự khác nhau (dựa trên bảng tần suất)
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
