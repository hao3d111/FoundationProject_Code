#ifndef HEADER_WRITER_HPP
#define HEADER_WRITER_HPP
#include <fstream>
#include <map>
inline void ghiHeader(std::ofstream &out,const std::map<unsigned char, unsigned int> &freq,int padding) {
    unsigned char pad = (unsigned char)padding;
    unsigned short n = (unsigned short)freq.size();
    out.write((char*)&pad, sizeof(pad));
    out.write((char*)&n, sizeof(n));
    for (auto &p : freq) {
        unsigned char c = p.first;
        unsigned int f = p.second;
        out.write((char*)&c, sizeof(c));
        out.write((char*)&f, sizeof(f));
    }
}
inline void docHeader(std::ifstream &in,std::map<unsigned char, unsigned int> &freq,int &padding) {
    unsigned char pad;
    unsigned short n;
    in.read((char*)&pad, sizeof(pad));
    in.read((char*)&n, sizeof(n));
    padding = pad;

    freq.clear();
    for (int i = 0; i < n; i++) {
        unsigned char c;
        unsigned int f;
        in.read((char*)&c, sizeof(c));
        in.read((char*)&f, sizeof(f));
        freq[c] = f;
    }
}
#endif
