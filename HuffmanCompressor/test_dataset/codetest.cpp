#include <iostream>
#include <vector>
#include <queue>
#include <cstdint>

struct Node {
    uint8_t byteVal;
    uint32_t freq;
    Node *left, *right;
    Node(uint8_t val, uint32_t f) : byteVal(val), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

int main() {
    std::priority_queue<Node*, std::vector<Node*>, Compare> minHeap;
    uint32_t freqTable[256] = {0};
    
    // Simulate frequency count
    freqTable['A'] = 100;
    freqTable['B'] = 45;
    freqTable['C'] = 15;
    
    for (int i = 0; i < 256; ++i) {
        if (freqTable[i] > 0) {
            minHeap.push(new Node(static_cast<uint8_t>(i), freqTable[i]));
        }
    }
    
    std::cout << "Min-Heap initialized with " << minHeap.size() << " unique bytes." << std::endl;
    return 0;
}