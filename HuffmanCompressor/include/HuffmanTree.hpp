#ifndef HUFFMAN_TREE_HPP
#define HUFFMAN_TREE_HPP

#include <string>
#include <array>
#include <vector>
#include "HuffmanCore.hpp"

class HuffmanTree {
private:
    HuffmanNode* root;

    void generate_codes_recursive(HuffmanNode* node, const std::string& current_code, std::array<std::string, 256>& code_table) {
        if (!node) return;

        if (node->is_leaf()) {
            code_table[static_cast<uint8_t>(node->byte_val)] = current_code.empty() ? "0" : current_code;
            return;
        }

        generate_codes_recursive(node->left, current_code + "0", code_table);
        generate_codes_recursive(node->right, current_code + "1", code_table);
    }

    void destroy_tree(HuffmanNode* node) {
        if (!node) return;
        destroy_tree(node->left);
        destroy_tree(node->right);
        delete node;
    }

public:
    HuffmanTree() : root(nullptr) {}

    ~HuffmanTree() {
        destroy_tree(root);
    }

    HuffmanNode* get_root() const {
        return root;
    }

    void build_tree(const std::array<uint64_t, 256>& freq_table) {
        MinHeap heap;

        for (int i = 0; i < 256; ++i) {
            if (freq_table[i] > 0) {
                heap.push(new HuffmanNode(static_cast<int16_t>(i), freq_table[i]));
            }
        }

        if (heap.empty()) {
            root = nullptr;
            return;
        }

        if (heap.size() == 1) {
            HuffmanNode* single = heap.pop();
            root = new HuffmanNode(-1, single->freq);
            root->left = single;
            return;
        }

        while (heap.size() > 1) {
            HuffmanNode* left = heap.pop();
            HuffmanNode* right = heap.pop();

            HuffmanNode* parent = new HuffmanNode(-1, left->freq + right->freq);
            parent->left = left;
            parent->right = right;

            heap.push(parent);
        }

        root = heap.pop();
    }

    std::array<std::string, 256> build_code_table() {
        std::array<std::string, 256> code_table;
        for (auto& s : code_table) s = "";

        if (root) {
            generate_codes_recursive(root, "", code_table);
        }

        return code_table;
    }
};

#endif
