#include "../include/UI.hpp"
#include "../include/Benchmark.hpp"
#include "../include/HeaderReader.hpp"
#include "../include/HuffmanEncoder.hpp"
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

void UI::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void UI::pauseScreen() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void UI::displayHeader() {
    std::cout << "=================================================================\n";
    std::cout << "     HUFFMAN FILE COMPRESSOR & DECOMPRESSOR TOOL (v1.0)          \n";
    std::cout << "     Project Team: Nam Luu, Huu Nguyen, Duong Hoang              \n";
    std::cout << "=================================================================\n\n";
}

void UI::displayMenu() {
    displayHeader();
    std::cout << "  [1] Compress File (.huff)\n";
    std::cout << "  [2] Decompress File (.huff)\n";
    std::cout << "  [3] Run Benchmark Test Suite\n";
    std::cout << "  [0] Exit Application\n";
    std::cout << "-----------------------------------------------------------------\n";
    std::cout << "Select an option (0-3): ";
}

void UI::run() {
    int choice = -1;
    while (choice != 0) {
        clearScreen();
        displayMenu();
        
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        switch (choice) {
            case 1:
                handleCompression();
                break;
            case 2:
                handleDecompression();
                break;
            case 3:
                handleBenchmarking();
                break;
            case 0:
                std::cout << "\nThank you for using the Huffman Compression Tool. Goodbye!\n";
                break;
            default:
                std::cout << "\nInvalid selection! Please try again.";
                pauseScreen();
                break;
        }
    }
}

void UI::handleCompression() {
    clearScreen();
    displayHeader();
    std::cout << "=== [1] FILE COMPRESSION ===\n\n";
    
    std::string inputPath;
    std::cout << "Enter source file path: ";
    std::cin >> inputPath;

    if (!fs::exists(inputPath) || fs::is_directory(inputPath)) {
        std::cout << "\n[ERROR] File does not exist or is a directory!\n";
        pauseScreen();
        return;
    }

    std::string outputPath = inputPath + ".huff";
    std::cout << "Target compressed path : " << outputPath << "\n\n";
    std::cout << "[INFO] Processing compression...\n";

    // Initialize Timer for execution time measurement
    Timer timer;
    timer.start();

    try {
        // Execute real compression flow
        HuffmanEncoder::compress(inputPath, outputPath);

        timer.stop(); // Stop timer

        // Compute performance stats via Profiler
        CompressionStats stats = Profiler::calculate_stats(inputPath, outputPath, timer.get_elapsed_milliseconds());

        std::cout << "\n[SUCCESS] File compression completed!";
        std::cout << "\n- Original Size  : " << stats.original_size_bytes << " Bytes (" << (stats.original_size_bytes / 1024.0) << " KB)";
        std::cout << "\n- Compressed Size: " << stats.compressed_size_bytes << " Bytes (" << (stats.compressed_size_bytes / 1024.0) << " KB)";
        std::cout << "\n- Space Saved    : " << std::fixed << std::setprecision(2) << stats.space_saved_percent << "%";
        std::cout << "\n- Execution Time : " << stats.execution_time_ms << " ms\n";
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] Compression failed: " << e.what() << "\n";
    }

    pauseScreen();
}

void UI::handleDecompression() {
    clearScreen();
    displayHeader();
    std::cout << "=== [2] FILE DECOMPRESSION ===\n\n";
    
    std::string inputPath;
    std::cout << "Enter .huff compressed file path: ";
    std::cin >> inputPath;

    if (!fs::exists(inputPath)) {
        std::cout << "\n[ERROR] Compressed file does not exist!\n";
        pauseScreen();
        return;
    }
    
    // Yêu cầu nhập tên tệp mới
    std::string customFileName;
    std::cout << "Enter desired output file name (e.g., restored_file.txt): ";
    std::cin >> customFileName;

    // Tách thư mục gốc và ghép với tên tệp mới
    fs::path compressedPath(inputPath);
    fs::path parentDirectory = compressedPath.parent_path();
    std::string outputPath;
    
    if (parentDirectory.empty()) {
        outputPath = customFileName; // Nếu chạy file ở thư mục hiện hành
    } else {
        outputPath = (parentDirectory / customFileName).string(); // Nối thư mục và tên file
    }
    
    std::cout << "Destination path set to: " << outputPath << "\n";
    std::cout << "\n[INFO] Processing decompression...\n";

    Timer timer;
    timer.start();

    try {
        HuffmanDecoder::decompress(inputPath, outputPath);
        
        timer.stop();

        uint64_t restored_size = Profiler::get_file_size(outputPath);

        std::cout << "\n[SUCCESS] File decompression completed!";
        std::cout << "\n- Restored File Size : " << restored_size << " Bytes (" << (restored_size / 1024.0) << " KB)";
        std::cout << "\n- Execution Time     : " << std::fixed << std::setprecision(2) << timer.get_elapsed_milliseconds() << " ms\n";
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] Decompression failed: " << e.what() << "\n";
    }

    pauseScreen();
}
void UI::handleBenchmarking() {
    clearScreen();
    displayHeader();
    std::cout << "=== [3] BENCHMARK TEST SUITE ===\n\n";
    
    std::string datasetDir = "./test_dataset/";
    std::cout << "Target Directory: " << datasetDir << "\n\n";

    std::cout << std::left 
              << std::setw(28) << "File Name" 
              << std::setw(15) << "Original (KB)" 
              << std::setw(15) << "Compressed" 
              << std::setw(12) << "Saved (%)" 
              << std::setw(10) << "Time (ms)" << "\n";
    std::cout << "-----------------------------------------------------------------------------------\n";

    if (fs::exists(datasetDir) && fs::is_directory(datasetDir)) {
        for (const auto& entry : fs::directory_iterator(datasetDir)) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();
                std::string filename = entry.path().filename().string();

                // Skip existing .huff or temporary binary test files
                if (entry.path().extension() == ".huff" || entry.path().extension() == ".bin") continue;

                std::string output_huff = filepath + ".test.huff";

                Timer timer;
                timer.start();

                try {
                    // Test algorithm performance on dataset entry
                    HuffmanEncoder::compress(filepath, output_huff);
                    timer.stop();

                    CompressionStats stats = Profiler::calculate_stats(filepath, output_huff, timer.get_elapsed_milliseconds());

                    std::cout << std::left 
                              << std::setw(28) << filename 
                              << std::setw(15) << std::fixed << std::setprecision(2) << (stats.original_size_bytes / 1024.0)
                              << std::setw(15) << (stats.compressed_size_bytes / 1024.0)
                              << std::setw(12) << stats.space_saved_percent
                              << std::setw(10) << stats.execution_time_ms << "\n";

                    // Cleanup temporary compressed file
                    if (fs::exists(output_huff)) fs::remove(output_huff);
                } catch (...) {
                    std::cout << std::left << std::setw(28) << filename << " [COMPRESSION ERROR]\n";
                }
            }
        }
    } else {
        std::cout << "Directory ./test_dataset/ not found or empty.\n";
    }
    
    pauseScreen();
}
