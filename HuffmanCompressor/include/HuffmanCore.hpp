#ifndef HUFFMAN_CORE_HPP
#define HUFFMAN_CORE_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <filesystem>

struct HuffmanNode {
    int16_t byte_val;
    uint64_t freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(int16_t val, uint64_t f)
        : byte_val(val), freq(f), left(nullptr), right(nullptr) {}

    bool is_leaf() const {
        return left == nullptr && right == nullptr;
    }
};

class MinHeap {
private:
    std::vector<HuffmanNode*> data;

    int parent(int i) const { return (i - 1) / 2; }
    int left_child(int i) const { return 2 * i + 1; }
    int right_child(int i) const { return 2 * i + 2; }

    void sift_up(int index) {
        while (index > 0 && data[index]->freq < data[parent(index)]->freq) {
            std::swap(data[index], data[parent(index)]);
            index = parent(index);
        }
    }

    void sift_down(int index) {
        int size = static_cast<int>(data.size());
        while (true) {
            int smallest = index;
            int l = left_child(index);
            int r = right_child(index);

            if (l < size && data[l]->freq < data[smallest]->freq) {
                smallest = l;
            }
            if (r < size && data[r]->freq < data[smallest]->freq) {
                smallest = r;
            }

            if (smallest != index) {
                std::swap(data[index], data[smallest]);
                index = smallest;
            } else {
                break;
            }
        }
    }

public:
    MinHeap() = default;

    explicit MinHeap(const std::vector<HuffmanNode*>& nodes) : data(nodes) {
        for (int i = (static_cast<int>(data.size()) / 2) - 1; i >= 0; --i) {
            sift_down(i);
        }
    }

    size_t size() const {
        return data.size();
    }

    bool empty() const {
        return data.empty();
    }

    void push(HuffmanNode* node) {
        data.push_back(node);
        sift_up(static_cast<int>(data.size()) - 1);
    }

    HuffmanNode* pop() {
        if (data.empty()) {
            throw std::out_of_range("MinHeap::pop(): Heap empty");
        }
        HuffmanNode* min_node = data[0];
        data[0] = data.back();
        data.pop_back();

        if (!data.empty()) {
            sift_down(0);
        }
        return min_node;
    }

    HuffmanNode* peek() const {
        if (data.empty()) return nullptr;
        return data[0];
    }
};

inline std::array<uint64_t, 256> compute_byte_frequencies(const std::string& filepath) {
    std::array<uint64_t, 256> freq_table = {0};
    std::ifstream file(std::filesystem::u8path(filepath), std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    constexpr size_t BUFFER_SIZE = 64 * 1024;
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    while (file.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE) || file.gcount() > 0) {
        std::streamsize bytes_read = file.gcount();
        for (std::streamsize i = 0; i < bytes_read; ++i) {
            freq_table[buffer[i]]++;
        }
    }

    return freq_table;
}

#endif
