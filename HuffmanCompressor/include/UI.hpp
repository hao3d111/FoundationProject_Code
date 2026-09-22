#ifndef UI_HPP
#define UI_HPP

#include <string>

class UI {
public:
    // Display main menu and manage application execution flow
    void run();

private:
    void displayHeader();
    void displayMenu();
    
    // Command handler stubs for connecting Huffman Core & Bitwise I/O modules
    void handleCompression();
    void handleDecompression();
    void handleBenchmarking();
    
    // UI Helpers
    void clearScreen();
    void pauseScreen();
};

#endif // UI_HPP