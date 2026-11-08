#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <chrono>
#include <filesystem>
#include <string>
#include <cstdint>

namespace fs = std::filesystem;

struct CompressionStats {
    uint64_t original_size_bytes = 0;
    uint64_t compressed_size_bytes = 0;
    double execution_time_ms = 0.0;
    double compression_ratio = 0.0;    // Size ratio (Compressed / Original * 100)
    double space_saved_percent = 0.0;  // Space saved percentage (100 - Compression Ratio)
};

class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;

public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        end_time = std::chrono::high_resolution_clock::now();
    }

    // Returns elapsed execution time in milliseconds (ms)
    double get_elapsed_milliseconds() const {
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
        return elapsed.count();
    }
};

class Profiler {
public:
    // Measures file size in bytes
    static uint64_t get_file_size(const std::string& filepath) {
        fs::path p = fs::u8path(filepath);
        if (fs::exists(p) && fs::is_regular_file(p)) {
            return fs::file_size(p);
        }
        return 0;
    }

    // Calculates compression performance metrics
    static CompressionStats calculate_stats(const std::string& input_path, 
                                            const std::string& output_path, 
                                            double time_ms) {
        CompressionStats stats;
        stats.original_size_bytes = get_file_size(input_path);
        stats.compressed_size_bytes = get_file_size(output_path);
        stats.execution_time_ms = time_ms;

        if (stats.original_size_bytes > 0) {
            stats.compression_ratio = (static_cast<double>(stats.compressed_size_bytes) / stats.original_size_bytes) * 100.0;
            stats.space_saved_percent = 100.0 - stats.compression_ratio;
        }

        return stats;
    }
};

#endif // BENCHMARK_HPP
