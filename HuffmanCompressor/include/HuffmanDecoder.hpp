#ifndef HUFFMAN_DECODER_HPP
#define HUFFMAN_DECODER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <filesystem>
#include "HuffmanCore.hpp"
#include "BitIO.hpp"
#include "HuffmanTree.hpp"

class HuffmanDecoder {
public:
    static void decompress(const std::string& input_path, const std::string& output_path) {
        std::ifstream in_file(std::filesystem::u8path(input_path), std::ios::binary);
        if (!in_file.is_open()) {
            throw std::runtime_error("Cannot open compressed file: " + input_path);
        }

        uint16_t distinct_symbols = 0;
        uint64_t total_bytes = 0;

        in_file.read(reinterpret_cast<char*>(&distinct_symbols), sizeof(distinct_symbols));
        in_file.read(reinterpret_cast<char*>(&total_bytes), sizeof(total_bytes));

        if (!in_file) {
            throw std::runtime_error("Cannot read file header.");
        }

        std::ofstream out_file(std::filesystem::u8path(output_path), std::ios::binary);
        if (!out_file.is_open()) {
            throw std::runtime_error("Cannot create output file: " + output_path);
        }

        if (total_bytes == 0) {
            return;
        }

        std::array<uint64_t, 256> freq_table = {0};
        for (uint16_t i = 0; i < distinct_symbols; ++i) {
            uint8_t byte_val;
            uint64_t freq;
            in_file.read(reinterpret_cast<char*>(&byte_val), sizeof(byte_val));
            in_file.read(reinterpret_cast<char*>(&freq), sizeof(freq));
            if (!in_file) {
                throw std::runtime_error("Cannot read frequency table from header.");
            }
            freq_table[byte_val] = freq;
        }

        HuffmanTree tree;
        tree.build_tree(freq_table);
        HuffmanNode* root = tree.get_root();

        if (root == nullptr) {
            throw std::runtime_error("Cannot rebuild Huffman tree from header.");
        }

        constexpr size_t BUFFER_SIZE = 64 * 1024;
        std::vector<uint8_t> write_buffer;
        write_buffer.reserve(BUFFER_SIZE);

        if (root->left == nullptr && root->right == nullptr) {
            uint8_t single_byte = static_cast<uint8_t>(root->byte_val);
            for (uint64_t i = 0; i < total_bytes; ++i) {
                write_buffer.push_back(single_byte);
                if (write_buffer.size() >= BUFFER_SIZE) {
                    out_file.write(reinterpret_cast<const char*>(write_buffer.data()), write_buffer.size());
                    write_buffer.clear();
                }
            }
            if (!write_buffer.empty()) {
                out_file.write(reinterpret_cast<const char*>(write_buffer.data()), write_buffer.size());
            }
            out_file.close();
            in_file.close();
            return;
        }

        BitReader bit_reader(in_file);
        HuffmanNode* current_node = root;
        uint64_t decoded_bytes = 0;

        while (decoded_bytes < total_bytes) {
            int bit = bit_reader.read_bit();
            if (bit == -1) {
                break;
            }

            current_node = (bit == 0) ? current_node->left : current_node->right;

            if (current_node == nullptr) {
                throw std::runtime_error("Invalid bit stream: reached null node in Huffman tree.");
            }

            if (current_node->is_leaf()) {
                write_buffer.push_back(static_cast<uint8_t>(current_node->byte_val));
                decoded_bytes++;
                current_node = root;

                if (write_buffer.size() >= BUFFER_SIZE) {
                    out_file.write(reinterpret_cast<const char*>(write_buffer.data()), write_buffer.size());
                    write_buffer.clear();
                }
            }
        }

        if (!write_buffer.empty()) {
            out_file.write(reinterpret_cast<const char*>(write_buffer.data()), write_buffer.size());
        }

        if (decoded_bytes != total_bytes) {
            throw std::runtime_error("Corrupted file: decoded byte count does not match header.");
        }

        out_file.close();
        in_file.close();
    }
};

#endif // HUFFMAN_DECODER_HPP
