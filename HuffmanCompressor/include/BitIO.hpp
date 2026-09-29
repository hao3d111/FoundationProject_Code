#ifndef BIT_IO_HPP
#define BIT_IO_HPP

#include <iostream>
#include <string>
#include <cstdint>
#include <stdexcept>


class BitWriter {
private:
    std::ostream& out_stream;
    uint8_t buffer;
    uint8_t bit_count;

public:
    explicit BitWriter(std::ostream& os) : out_stream(os), buffer(0), bit_count(0) {}

    void write_bit(uint8_t bit) {
        buffer = (buffer << 1) | (bit & 1);
        bit_count++;
        if (bit_count == 8) {
            out_stream.put(static_cast<char>(buffer));
            buffer = 0;
            bit_count = 0;
        }
    }

    void write_bits(const std::string& bitstring) {
        for (char ch : bitstring) {
            if (ch == '1') {
                write_bit(1);
            } else if (ch == '0') {
                write_bit(0);
            } else {
                throw std::invalid_argument("Invalid bit character in bitstring!");
            }
        }
    }


    uint8_t flush() {
        uint8_t padding = 0;
        if (bit_count > 0) {
            padding = 8 - bit_count;
            buffer <<= padding; 
            out_stream.put(static_cast<char>(buffer));
            buffer = 0;
            bit_count = 0;
        }
        return padding;
    }

    uint8_t get_pending_bits() const {
        return bit_count;
    }
};

class BitReader {
private:
    std::istream& in_stream;
    uint8_t buffer;
    uint8_t bits_left;

public:
    explicit BitReader(std::istream& is) : in_stream(is), buffer(0), bits_left(0) {}

    int read_bit() {
        if (bits_left == 0) {
            char byte_read;
            if (!in_stream.get(byte_read)) {
                return -1; 
            }
            buffer = static_cast<uint8_t>(byte_read);
            bits_left = 8;
        }
        bits_left--;
        return (buffer >> bits_left) & 1;
    }

    void reset() {
        buffer = 0;
        bits_left = 0;
    }
};

#endif // BIT_IO_HPP
