#include "HuffmanDecoder.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;
void get_codes(HuffmanNode* node, string code, map<int, string>& codes) {
    if (node == nullptr) return;
    if (node->is_leaf()) {
        codes[(uint8_t)node->byte_val] = code;
        return;
    }
    get_codes(node->left, code + "0", codes);
    get_codes(node->right, code + "1", codes);
}
void make_file(string data, string path) {
    array<uint64_t, 256> freq = {0};
    for (unsigned char c : data) freq[c]++;

    uint16_t distinct = 0;
    int only = -1;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            distinct++;
            only = i;
        }
    }
    if (distinct == 1) {
        int dummy = (only + 1) % 256;
        freq[dummy] = 1;
        distinct = 2;
    }
    uint64_t total = data.size();
    ofstream out(path, ios::binary);
    out.write((char*)&distinct, sizeof(distinct));
    out.write((char*)&total, sizeof(total));
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            uint8_t b = i;
            out.write((char*)&b, sizeof(b));
            out.write((char*)&freq[i], sizeof(uint64_t));
        }
    }
    if (total == 0) return;
    HuffmanTree tree;
    tree.build_tree(freq);
    map<int, string> codes;
    get_codes(tree.get_root(), "", codes);
    string bits = "";
    for (unsigned char c : data) bits += codes[c];
    while (bits.size() % 8 != 0) bits += "0";
    for (size_t i = 0; i < bits.size(); i += 8) {
        uint8_t byte = 0;
        for (int k = 0; k < 8; k++) {
            byte = (byte << 1) | (bits[i + k] - '0');
        }
        out.put((char)byte);
    }
}
string read_file(string path) {
    ifstream f(path, ios::binary);
    string s((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    return s;
}
void test(string name, string data) {
    make_file(data, "test.huf");
    try {
        HuffmanDecoder::decompress("test.huf", "test.out");
        string result = read_file("test.out");
        if (result == data) cout << "PASS: " << name << endl;
        else cout << "FAIL: " << name << endl;
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
    ofstream bad("bad.huf", ios::binary);
    bad.write(full.data(), full.size() - 1);
    bad.close();
    try {
        HuffmanDecoder::decompress("bad.huf", "bad.out");
        cout << "FAIL: file bi cat (khong bao loi)" << endl;
    } catch (exception& e) {
        cout << "PASS: file bi cat - " << e.what() << endl;
    }
    try {
        HuffmanDecoder::decompress("khong_co.huf", "x.out");
        cout << "FAIL: file khong ton tai (khong bao loi)" << endl;
    } catch (exception& e) {
        cout << "PASS: file khong ton tai - " << e.what() << endl;
    }
    return 0;
}