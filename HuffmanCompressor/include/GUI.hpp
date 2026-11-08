#ifndef GUI_HPP
#define GUI_HPP

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <filesystem>
#include "../resources/resource.h"
#include "HuffmanEncoder.hpp"
#include "HuffmanDecoder.hpp"
#include "Benchmark.hpp"

// Custom window messages for worker thread notifications
#define WM_APP_COMPRESS_FINISH    (WM_APP + 101)
#define WM_APP_DECOMPRESS_FINISH  (WM_APP + 102)
#define WM_APP_BENCHMARK_ITEM     (WM_APP + 103)
#define WM_APP_BENCHMARK_FINISH   (WM_APP + 104)

struct CompressResult {
    bool success;
    std::wstring errorMsg;
    uint64_t origSize;
    uint64_t compSize;
    double spaceSaved;
    double timeMs;
    std::wstring outputPath;
};

struct DecompressResult {
    bool success;
    std::wstring errorMsg;
    uint64_t restoredSize;
    double timeMs;
    std::wstring outputPath;
};

struct BenchmarkItemData {
    int index;
    std::wstring fileName;
    double origKb;
    double compKb;
    double savedPercent;
    double timeMs;
    bool success;
    std::wstring statusText;
};

struct BenchmarkSummaryData {
    int totalFiles;
    int successCount;
    uint64_t totalOrigBytes;
    uint64_t totalCompBytes;
    double totalTimeMs;
    double avgSavedPercent;
};

class GUI {
public:
    static int run(HINSTANCE hInstance, int nCmdShow);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Initialization helpers
    void initDPI();
    void createFonts();
    void createControls(HWND hWnd);
    void setupTabs(HWND hWnd);
    void setupListView(HWND hList);
    void showTab(int tabIndex);

    // Event handlers
    void onBrowseCompressSource();
    void onBrowseCompressDest();
    void onExecuteCompress();

    void onBrowseDecompressSource();
    void onBrowseDecompressDest();
    void onExecuteDecompress();

    void onBrowseBenchmarkDir();
    void onExecuteBenchmark();
    void onCleanBenchmarkFiles();

    void onOpenFileFolder(const std::wstring& path);
    void handleFileDrop(HDROP hDrop);

    // Background worker completion handlers
    void onCompressFinished(const CompressResult& res);
    void onDecompressFinished(const DecompressResult& res);
    void onBenchmarkItem(const BenchmarkItemData& item);
    void onBenchmarkFinished(const BenchmarkSummaryData& summary);

    // Utilities
    void setControlText(int ctrlId, const std::wstring& text);
    std::wstring getControlText(int ctrlId);
    void updateStatusBar(const std::wstring& text);
    void setControlsEnabled(bool enabled);

    static std::wstring formatSize(uint64_t bytes);
    static std::wstring formatTime(double ms);
    static std::wstring utf8ToWide(const std::string& str);
    static std::string wideToUtf8(const std::wstring& wstr);

    // Members
    HWND m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;

    HFONT m_hFontDefault = nullptr;
    HFONT m_hFontBold = nullptr;
    HFONT m_hFontTitle = nullptr;
    HFONT m_hFontSubtitle = nullptr;
    HFONT m_hFontStats = nullptr;

    HBRUSH m_hBrushHeader = nullptr;
    HBRUSH m_hBrushBg = nullptr;

    int m_currentTab = 0;
    std::atomic<bool> m_isBusy{false};
    std::thread m_workerThread;

    std::wstring m_lastCompressedOutput;
    std::wstring m_lastDecompressedOutput;

    // Control groups per tab for visibility toggling
    std::vector<HWND> m_tabCompressCtrls;
    std::vector<HWND> m_tabDecompressCtrls;
    std::vector<HWND> m_tabBenchmarkCtrls;

    static GUI* s_instance;
};

#endif // GUI_HPP
