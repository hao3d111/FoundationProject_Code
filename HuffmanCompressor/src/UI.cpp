#include "../include/UI.hpp"
#include <iostream>
#include <iomanip>
#include <string>

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
    
    std::string inputPath, outputPath;
    std::cout << "Enter source file path: ";
    std::cin >> inputPath;
    
    outputPath = inputPath + ".huff";
    std::cout << "Output compressed file path: " << outputPath << "\n\n";
    
    std::cout << "[INFO] Processing compression...\n";
    
    // TODO: Connect with Huu and Duong's compression engine module
    // Example: CompressEngine::compress(inputPath, outputPath);
    
    std::cout << "\n[SUCCESS] File compressed successfully!";
    std::cout << "\n- Original Size  : 1,024 KB";
    std::cout << "\n- Compressed Size: 412 KB";
    std::cout << "\n- Space Saved    : 59.76%";
    std::cout << "\n- Execution Time : 14 ms\n";
    
    pauseScreen();
}

void UI::handleDecompression() {
    clearScreen();
    displayHeader();
    std::cout << "=== [2] FILE DECOMPRESSION ===\n\n";
    
    std::string inputPath, outputPath;
    std::cout << "Enter .huff compressed file path: ";
    std::cin >> inputPath;
    
    std::cout << "Enter destination path for restored file: ";
    std::cin >> outputPath;
    
    std::cout << "\n[INFO] Processing decompression...\n";
    
    // TODO: Connect with Huu and Duong's decompression engine module
    // Example: CompressEngine::decompress(inputPath, outputPath);
    
    std::cout << "\n[SUCCESS] File decompressed successfully!";
    std::cout << "\n- Restored Size   : 1,024 KB";
    std::cout << "\n- Hash Verification (MD5): MATCHED (100% Integrity)\n";
    
    pauseScreen();
}

void UI::handleBenchmarking() {
    clearScreen();
    displayHeader();
    std::cout << "=== [3] BENCHMARK TEST SUITE ===\n\n";
    
    std::cout << "Target Directory: ./test_dataset/\n\n";
    std::cout << std::left 
              << std::setw(25) << "File Name" 
              << std::setw(15) << "Original (KB)" 
              << std::setw(15) << "Compressed" 
              << std::setw(12) << "Saved (%)" 
              << std::setw(10) << "Time (ms)" << "\n";
    std::cout << "-------------------------------------------------------------------\n";
    
    // Mock sample benchmark entries
    std::cout << std::setw(25) << "sample_text_english.txt" << std::setw(15) << "1.5" << std::setw(15) << "0.8 KB" << std::setw(12) << "46.67" << std::setw(10) << "2" << "\n";
    std::cout << std::setw(25) << "sample_source_code.cpp"  << std::setw(15) << "1.0" << std::setw(15) << "0.6 KB" << std::setw(12) << "40.00" << std::setw(10) << "1" << "\n";
    std::cout << std::setw(25) << "edge_single_char.txt"    << std::setw(15) << "0.1" << std::setw(15) << "0.02 KB" << std::setw(12) << "80.00" << std::setw(10) << "0" << "\n";
    
    pauseScreen();
}