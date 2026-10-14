#include "HuffmanDecoder.hpp"
#include "HuffmanEncoder.hpp"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Hàm tạo file nén gọi trực tiếp mô-đun chuẩn của nhóm
void make_file(string data, string path) {
    string temp_input = "temp_input.txt";
    {
        ofstream out(temp_input, ios::binary);
        out.write(data.data(), data.size());
    }
    try {
        HuffmanEncoder::compress(temp_input, path);
    } catch (...) {
        // Xử lý file rỗng hoặc lỗi ngoại lệ nếu có
        ofstream out(path, ios::binary);
        uint16_t zero_sym = 0;
        uint64_t zero_bytes = data.size();
        out.write((char*)&zero_sym, sizeof(zero_sym));
        out.write((char*)&zero_bytes, sizeof(zero_bytes));
    }
}

string read_file(string path) {
    ifstream f(path, ios::binary);
    if (!f.is_open()) return "";
    string s((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    return s;
}

void test(string name, string data) {
    make_file(data, "test.huf");
    try {
        HuffmanDecoder::decompress("test.huf", "test.out");
        string result = read_file("test.out");
        if (result == data) cout << "PASS: " << name << endl;
        else cout << "FAIL: " << name << " (Data mismatch)" << endl;
    } catch (exception& e) {
        cout << "FAIL: " << name << " - " << e.what() << endl;
    }
}

int main() {
    test("aaabbc", "aaabbc");
    test("abababab", "abababab");
    test("hello world", "hello world");
    test("rong", "");
    test("1 ky tu", "aaaaa");
    test("du 8 bit", "aabbccdd");
    test("can padding", "abcabcabcab");
    test("byte 0 va 255", string("\x00\xFF\x00\xFF\xFF", 5));
    test("file lon 70000", string(70000, 'x') + "yz");

    make_file("aaabbbcccddd", "test.huf");
    string full = read_file("test.huf");
    
    if (full.size() > 1) {
        ofstream bad("bad.huf", ios::binary);
        bad.write(full.data(), full.size() - 1);
        bad.close();
        try {
            HuffmanDecoder::decompress("bad.huf", "bad.out");
            cout << "FAIL: file bi cat (khong bao loi)" << endl;
        } catch (exception& e) {
            cout << "PASS: file bi cat - " << e.what() << endl;
        }
    }

    try {
        HuffmanDecoder::decompress("khong_co.huf", "x.out");
        cout << "FAIL: file khong ton tai (khong bao loi)" << endl;
    } catch (exception& e) {
        cout << "PASS: file khong ton tai - " << e.what() << endl;
    }

    return 0;
}
