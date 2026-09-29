#include "../include/BitIO.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

// Function to test writing and reading individual bits
void test_bit_by_bit_io() {
    std::string test_filename = "test_bits.bin";
    
    // 1. Write bit sequence: "10110011" (Binary = 0xB3 = 179)
    {
        std::ofstream out(test_filename, std::ios::binary);
        BitWriter writer(out);
        writer.write_bits("10110011");
        writer.flush();
        out.close();
    }

    // 2. Read back and verify bit sequence
    {
        std::ifstream in(test_filename, std::ios::binary);
        BitReader reader(in);
        std::string read_bits = "";

        for (int i = 0; i < 8; ++i) {
            int bit = reader.read_bit();
            if (bit != -1) {
                read_bits += std::to_string(bit);
            }
        }
        in.close();

        assert(read_bits == "10110011");
        std::cout << "[PASS] Unit Test BitIO: Write & Read '10110011' matched!\n";
    }
}

// Function to test binary file edge cases
void test_large_bit_stream() {
    std::string test_filename = "test_stream.bin";
    std::string pattern = "110010101111000010101010"; // 24 bits = 3 bytes

    {
        std::ofstream out(test_filename, std::ios::binary);
        BitWriter writer(out);
        writer.write_bits(pattern);
        writer.flush();
        out.close();
    }

    {
        std::ifstream in(test_filename, std::ios::binary);
        BitReader reader(in);
        std::string result = "";

        int bit;
        while ((bit = reader.read_bit()) != -1) {
            result += std::to_string(bit);
        }
        in.close();

        assert(result == pattern);
        std::cout << "[PASS] Unit Test BitIO: Multi-byte bitstream matched!\n";
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   RUNNING UNIT TESTS FOR BITWISE I/O   " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_bit_by_bit_io();
        test_large_bit_stream();
        std::cout << "\nALL UNIT TESTS PASSED SUCCESSFULLY!\n";
    } catch (const std::exception& e) {
        std::cerr << "\n[FAIL] Unit Test Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}