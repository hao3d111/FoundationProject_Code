#include "../include/UI.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
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

    // Check if input file exists
    if (!fs::exists(inputPath) || fs::is_directory(inputPath)) {
        std::cout << "\n[ERROR] File does not exist or is a directory!\n";
        pauseScreen();
        return;
    }

    uint64_t originalSize = fs::file_size(inputPath);
    std::string outputPath = inputPath + ".huff";

    std::cout << "Target compressed path : " << outputPath << "\n";
    std::cout << "Original File Size     : " << originalSize << " Bytes\n\n";
    std::cout << "[INFO] Compressing file...\n";

    // TODO: Connect with Huu and Duong's compression logic in Week 4/5
    
    std::cout << "\n[SUCCESS] File compression completed!\n";
    pauseScreen();
}

void UI::handleDecompression() {
    clearScreen();
    displayHeader();
    std::cout << "=== [2] FILE DECOMPRESSION ===\n\n";
    
    std::string inputPath, outputPath;
    std::cout << "Enter .huff compressed file path: ";
    std::cin >> inputPath;

    if (!fs::exists(inputPath)) {
        std::cout << "\n[ERROR] Compressed file does not exist!\n";
        pauseScreen();
        return;
    }
    
    std::cout << "Enter destination path for restored file: ";
    std::cin >> outputPath;
    
    std::cout << "\n[INFO] Decompressing file...\n";

    // TODO: Connect with Huu and Duong's decompression logic in Week 4/5
    
    std::cout << "\n[SUCCESS] File decompression completed!\n";
    pauseScreen();
}

void UI::handleBenchmarking() {
    clearScreen();
    displayHeader();
    std::cout << "=== [3] BENCHMARK TEST SUITE ===\n\n";
    
    std::string datasetDir = "./test_dataset/";
    std::cout << "Scanning directory: " << datasetDir << "\n\n";

    std::cout << std::left 
              << std::setw(25) << "File Name" 
              << std::setw(15) << "Original (Bytes)" 
              << std::setw(15) << "Status" << "\n";
    std::cout << "-------------------------------------------------------------------\n";

    if (fs::exists(datasetDir) && fs::is_directory(datasetDir)) {
        for (const auto& entry : fs::directory_iterator(datasetDir)) {
            if (entry.is_regular_file()) {
                std::cout << std::setw(25) << entry.path().filename().string() 
                          << std::setw(15) << entry.file_size() 
                          << std::setw(15) << "Ready to Test" << "\n";
            }
        }
    } else {
        std::cout << "Directory ./test_dataset/ not found or empty.\n";
    }
    
    pauseScreen();
}
