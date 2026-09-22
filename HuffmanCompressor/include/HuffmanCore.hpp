#ifndef HUFFMAN_CORE_HPP
#define HUFFMAN_CORE_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <array>
#include <utility>

// ============================================================================
// YÊU CẦU 1: HuffmanNode, MinHeap tự cài đặt và Hàm đếm tần số byte
// ============================================================================

/**
 * @brief Đại diện cho một nút trên Cây Huffman.
 * Nút lá lưu trữ giá trị byte (0 - 255). Nút nội bộ lưu byte_val = -1.
 */
struct HuffmanNode {
    int16_t byte_val;     // 0 - 255 nếu là nút lá; -1 nếu là nút trung gian
    uint64_t freq;        // Tần suất xuất hiện của byte
    HuffmanNode* left;    // Con trỏ nhánh trái (bit 0)
    HuffmanNode* right;   // Con trỏ nhánh phải (bit 1)

    HuffmanNode(int16_t val, uint64_t f)
        : byte_val(val), freq(f), left(nullptr), right(nullptr) {}

    bool is_leaf() const {
        return left == nullptr && right == nullptr;
    }
};

/**
 * @brief Cấu trúc Đống nhị phân tối thiểu (Binary Min-Heap) tự cài đặt.
 * Đảm bảo độ phức tạp O(log K) cho thao tác chèn và trích xuất.
 */
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

    // Khởi tạo Heap từ mảng với độ phức tạp O(K)
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
            throw std::out_of_range("MinHeap::pop(): Heap rỗng.");
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

/**
 * @brief Đọc file theo dạng nhị phân (buffer stream) và đếm tần số xuất hiện của từng byte (0 - 255).
 * Độ phức tạp thời gian: O(N) với N là dung lượng file.
 * Độ phức tạp không gian: O(1) (mảng tĩnh 256 phần tử).
 */
inline std::array<uint64_t, 256> compute_byte_frequencies(const std::string& filepath) {
    std::array<uint64_t, 256> freq_table = {0};
    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Không thể mở file để đếm tần số: " + filepath);
    }

    constexpr size_t BUFFER_SIZE = 64 * 1024; // Đọc khối 64 KB tối ưu I/O đĩa
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    while (file.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE) || file.gcount() > 0) {
        std::streamsize bytes_read = file.gcount();
        for (std::streamsize i = 0; i < bytes_read; ++i) {
            freq_table[buffer[i]]++;
        }
    }

    return freq_table;
}

// ============================================================================
// YÊU CẦU 2: BitWriter (Gom bit thành byte) & BitReader (Tách byte thành bit)
// ============================================================================

/**
 * @brief Gom từng bit riêng lẻ vào bộ đệm 8 bit và ghi ra file khi đủ 1 byte.
 */
class BitWriter {
private:
    std::ostream& out_stream;
    uint8_t buffer;     // Lưu trữ tạm thời các bit chưa gom đủ 1 byte
    uint8_t bit_count;  // Số lượng bit đang có trong buffer (0 - 7)

public:
    explicit BitWriter(std::ostream& os) : out_stream(os), buffer(0), bit_count(0) {}

    // Ghi một bit duy nhất (0 hoặc 1)
    void write_bit(uint8_t bit) {
        buffer = (buffer << 1) | (bit & 1);
        bit_count++;
        if (bit_count == 8) {
            out_stream.put(static_cast<char>(buffer));
            buffer = 0;
            bit_count = 0;
        }
    }

    // Ghi một chuỗi bit dạng string (vd: "010110")
    void write_bits(const std::string& bitstring) {
        for (char ch : bitstring) {
            if (ch == '1') {
                write_bit(1);
            } else if (ch == '0') {
                write_bit(0);
            } else {
                throw std::invalid_argument("Ký tự bit không hợp lệ (chỉ chấp nhận '0' hoặc '1').");
            }
        }
    }

    // Đẩy nốt các bit còn thừa ra file khi kết thúc ghi (padding số 0 vào cuối)
    void flush() {
        if (bit_count > 0) {
            buffer <<= (8 - bit_count); // Độn thêm (8 - bit_count) bit 0 vào vị trí thấp
            out_stream.put(static_cast<char>(buffer));
            buffer = 0;
            bit_count = 0;
        }
    }
};

/**
 * @brief Đọc từng byte từ stream và trích xuất tuần tự từng bit từ MSB xuống LSB.
 */
class BitReader {
private:
    std::istream& in_stream;
    uint8_t buffer;     // Byte hiện tại đang đọc
    uint8_t bits_left;  // Số bit còn lại chưa trích xuất trong buffer (0 - 8)

public:
    explicit BitReader(std::istream& is) : in_stream(is), buffer(0), bits_left(0) {}

    /**
     * @brief Đọc và trả về bit tiếp theo (0 hoặc 1).
     * @return 0 hoặc 1 nếu đọc thành công; -1 nếu gặp điểm kết thúc file (EOF).
     */
    int read_bit() {
        if (bits_left == 0) {
            char byte_read;
            if (!in_stream.get(byte_read)) {
                return -1; // Hết dữ liệu
            }
            buffer = static_cast<uint8_t>(byte_read);
            bits_left = 8;
        }
        bits_left--;
        return (buffer >> bits_left) & 1;
    }
};

#endif // HUFFMAN_CORE_HPP
