#ifndef HUFFMAN_DECODER_HPP
#define HUFFMAN_DECODER_HPP
#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <cstdint>
#include <stdexcept>
#include "HuffmanCore.hpp"
#include "BitIO.hpp"
#include "HuffmanTree.hpp"
class HuffmanDecoder {
public:
    static void readHeader(
        std::ifstream& in_file,
        uint16_t& distinct_symbols,
        uint64_t& total_bytes,
        std::array<uint64_t, 256>& freq_table
    ) {
        in_file.read(
            reinterpret_cast<char*>(&distinct_symbols),
            sizeof(distinct_symbols)
        );
        in_file.read(
            reinterpret_cast<char*>(&total_bytes),
            sizeof(total_bytes)
        );
        if (!in_file) {
            throw std::runtime_error("Cannot read Header.");
        }
        freq_table.fill(0);
        for (uint16_t i = 0; i < distinct_symbols; ++i) {
            uint8_t byte_val;
            uint64_t freq;
            in_file.read(
                reinterpret_cast<char*>(&byte_val),
                sizeof(byte_val)
            );
            in_file.read(
                reinterpret_cast<char*>(&freq),
                sizeof(freq)
            );
            if (!in_file) {
                throw std::runtime_error(
                    "Cannot read frequency table."
                );
            }
            freq_table[byte_val] = freq;
        }
    }
  static void decompress(
        const std::string& input_path,
        const std::string& output_path
    ) {
        std::ifstream in_file(
            input_path,
            std::ios::binary
        );
        if (!in_file.is_open()) {
            throw std::runtime_error(
                "Cannot open compressed file: " + input_path
            );
        }
        uint16_t distinct_symbols = 0;
        uint64_t total_bytes = 0;
        std::array<uint64_t, 256> freq_table{};
        readHeader(
            in_file,
            distinct_symbols,
            total_bytes,
            freq_table
        );
        std::ofstream out_file(
            output_path,
            std::ios::binary
        );
        if (!out_file.is_open()) {
            throw std::runtime_error(
                "Cannot create output file: " + output_path
            );
        }
        if (total_bytes == 0) {
            return;
        }
        HuffmanTree tree;
        tree.build_tree(freq_table);
        auto root = tree.get_root();
        if (root == nullptr) {
            throw std::runtime_error(
                "Cannot rebuild Huffman tree."
            );
        }
        if (root->left == nullptr &&
            root->right == nullptr) {
            for (uint64_t i = 0;
                 i < total_bytes;
                 ++i) {

                out_file.put(
                    static_cast<char>(root->byte)
                );
            }
            out_file.close();
            in_file.close();
            return;
        }
        BitReader reader(in_file);
        auto current = root;
        uint64_t decoded_bytes = 0;
        while (decoded_bytes < total_bytes) {
            int bit = reader.readBit();
            if (bit == -1) {
                throw std::runtime_error(
                    "Unexpected end of compressed file."
                );
            }
            if (bit == 0) {
                current = current->left;
            }
            else {
                current = current->right;
            }
            if (current == nullptr) {
                throw std::runtime_error(
                    "Invalid Huffman data."
                );
            }
            if (current->left == nullptr &&
                current->right == nullptr) {
                out_file.put(
                    static_cast<char>(current->byte)
                );
                decoded_bytes++;
              current = root;
            }
        }
        out_file.close();
        in_file.close();
    }
};

#endif