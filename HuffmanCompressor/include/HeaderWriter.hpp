#ifndef HEADER_WRITER_HPP
#define HEADER_WRITER_HPP

#include <fstream>
#include <array>
#include <cstdint>

inline void ghiHeader(std::ofstream &out, const std::array<uint64_t, 256> &freq, int padding) {
    uint8_t pad = static_cast<uint8_t>(padding);
    
    uint16_t unique_count = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) unique_count++;
    }

    out.write(reinterpret_cast<char*>(&pad), sizeof(pad));
    out.write(reinterpret_cast<char*>(&unique_count), sizeof(unique_count));

    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            uint8_t byte_val = static_cast<uint8_t>(i);
            uint64_t f = freq[i];
            out.write(reinterpret_cast<char*>(&byte_val), sizeof(byte_val));
            out.write(reinterpret_cast<char*>(&f), sizeof(f));
        }
    }
}

inline void docHeader(std::ifstream &in, std::array<uint64_t, 256> &freq, int &padding) {
    uint8_t pad;
    uint16_t unique_count;

    in.read(reinterpret_cast<char*>(&pad), sizeof(pad));
    in.read(reinterpret_cast<char*>(&unique_count), sizeof(unique_count));
    padding = pad;

    freq.fill(0);
    for (uint16_t i = 0; i < unique_count; ++i) {
        uint8_t byte_val;
        uint64_t f;
        in.read(reinterpret_cast<char*>(&byte_val), sizeof(byte_val));
        in.read(reinterpret_cast<char*>(&f), sizeof(f));
        freq[byte_val] = f;
    }
}

#endif
