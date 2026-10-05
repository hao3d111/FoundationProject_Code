#ifndef HUFFMAN_ENCODER_HPP
#define HUFFMAN_ENCODER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "HuffmanCore.hpp"
#include "BitIO.hpp"
#include "HuffmanTree.hpp"

class HuffmanEncoder {
public:
    static void compress(const std::string& input_path, const std::string& output_path) {
        auto freq_table = compute_byte_frequencies(input_path);

        uint16_t distinct_symbols = 0;
        uint64_t total_bytes = 0;
        for (int i = 0; i < 256; ++i) {
            if (freq_table[i] > 0) {
                distinct_symbols++;
                total_bytes += freq_table[i];
            }
        }

        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file.is_open()) {
            throw std::runtime_error("Cannot create output file: " + output_path);
        }

        out_file.write(reinterpret_cast<const char*>(&distinct_symbols), sizeof(distinct_symbols));
        out_file.write(reinterpret_cast<const char*>(&total_bytes), sizeof(total_bytes));

        if (total_bytes == 0) {
            return;
        }

        for (int i = 0; i < 256; ++i) {
            if (freq_table[i] > 0) {
                uint8_t byte_val = static_cast<uint8_t>(i);
                uint64_t freq = freq_table[i];
                out_file.write(reinterpret_cast<const char*>(&byte_val), sizeof(byte_val));
                out_file.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
            }
        }

        HuffmanTree tree;
        tree.build_tree(freq_table);
        auto code_table = tree.build_code_table();

        std::ifstream in_file(input_path, std::ios::binary);
        if (!in_file.is_open()) {
            throw std::runtime_error("Cannot open input file: " + input_path);
        }

        BitWriter bit_writer(out_file);
        constexpr size_t BUFFER_SIZE = 64 * 1024;
        std::vector<uint8_t> buffer(BUFFER_SIZE);

        while (in_file.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE) || in_file.gcount() > 0) {
            std::streamsize bytes_read = in_file.gcount();
            for (std::streamsize i = 0; i < bytes_read; ++i) {
                bit_writer.write_bits(code_table[buffer[i]]);
            }
        }

        bit_writer.flush();
    }
};

#endif
