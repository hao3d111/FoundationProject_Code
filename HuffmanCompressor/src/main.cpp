#include <windows.h>
#include <iostream>
#include <string>
#include "../include/GUI.hpp"
#include "../include/UI.hpp"

int main(int argc, char* argv[]) {
    // Check if user requested CLI mode via command line flag
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--cli" || arg == "-c" || arg == "-cli") {
            // Re-attach to parent console so terminal I/O works in -mwindows subsystem
            if (AttachConsole(ATTACH_PARENT_PROCESS)) {
                freopen("CONOUT$", "w", stdout);
                freopen("CONOUT$", "w", stderr);
                freopen("CONIN$", "r", stdin);
            }
            UI app;
            app.run();
            return 0;
        } else if (arg == "--help" || arg == "-h") {
            if (AttachConsole(ATTACH_PARENT_PROCESS)) {
                freopen("CONOUT$", "w", stdout);
                freopen("CONOUT$", "w", stderr);
                freopen("CONIN$", "r", stdin);
            }
            std::cout << "Huffman File Compressor & Decompressor (v1.0)\n";
            std::cout << "Usage:\n";
            std::cout << "  HuffmanTool.exe          Launch Graphical User Interface (GUI)\n";
            std::cout << "  HuffmanTool.exe --cli    Launch Command Line Interface (CLI)\n";
            std::cout << "  HuffmanTool.exe --help   Show this help message\n";
            return 0;
        }
    }

    // Default to modern Graphical User Interface (GUI)
    return GUI::run(GetModuleHandleW(NULL), SW_SHOWNORMAL);
}
