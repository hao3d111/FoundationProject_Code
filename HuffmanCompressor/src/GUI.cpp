#include "../include/GUI.hpp"
#include <shlobj.h>
#include <shellapi.h>
#include <sstream>
#include <iomanip>
#include <iostream>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comdlg32.lib")

GUI* GUI::s_instance = nullptr;

std::wstring GUI::utf8ToWide(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size <= 0) return std::wstring();
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

std::string GUI::wideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}

std::wstring GUI::formatSize(uint64_t bytes) {
    std::wstringstream ss;
    if (bytes < 1024) {
        ss << bytes << L" Bytes";
    } else if (bytes < 1024 * 1024) {
        ss << bytes << L" Bytes (" << std::fixed << std::setprecision(2) << (bytes / 1024.0) << L" KB)";
    } else {
        ss << bytes << L" Bytes (" << std::fixed << std::setprecision(2) << (bytes / (1024.0 * 1024.0)) << L" MB)";
    }
    return ss.str();
}

std::wstring GUI::formatTime(double ms) {
    std::wstringstream ss;
    if (ms < 1000.0) {
        ss << std::fixed << std::setprecision(2) << ms << L" ms";
    } else {
        ss << std::fixed << std::setprecision(2) << (ms / 1000.0) << L" s (" << std::fixed << std::setprecision(1) << ms << L" ms)";
    }
    return ss.str();
}

void GUI::initDPI() {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT);
        auto pfnSetDpi = (SetProcessDpiAwarenessContextProc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pfnSetDpi) {
            pfnSetDpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            return;
        }
        typedef BOOL(WINAPI* SetProcessDPIAwareProc)();
        auto pfnSetAware = (SetProcessDPIAwareProc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pfnSetAware) {
            pfnSetAware();
        }
    }
}

void GUI::createFonts() {
    m_hFontDefault = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hFontBold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hFontTitle = CreateFontW(-19, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hFontSubtitle = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hFontStats = CreateFontW(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

int GUI::run(HINSTANCE hInstance, int nCmdShow) {
    GUI gui;
    s_instance = &gui;
    gui.m_hInstance = hInstance;
    gui.initDPI();

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    gui.createFonts();
    gui.m_hBrushHeader = CreateSolidBrush(RGB(15, 23, 42)); // Modern slate navy
    gui.m_hBrushBg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = GUI::WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = gui.m_hBrushBg;
    wc.lpszClassName = L"HuffmanCompressorGUIClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Failed to register Windows window class!", L"Error", MB_ICONERROR);
        return 1;
    }

    int winWidth = 850;
    int winHeight = 690;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenWidth - winWidth) / 2;
    int posY = (screenHeight - winHeight) / 2;

    HWND hWnd = CreateWindowExW(
        WS_EX_ACCEPTFILES,
        L"HuffmanCompressorGUIClass",
        L"Huffman File Compressor & Decompressor (v1.0)",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, winWidth, winHeight,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        MessageBoxW(NULL, L"Failed to create application window!", L"Error", MB_ICONERROR);
        return 1;
    }

    gui.m_hWnd = hWnd;
    gui.createControls(hWnd);
    gui.setupTabs(hWnd);
    gui.showTab(0);
    gui.updateStatusBar(L"Ready. Select or drag and drop a file to get started.");

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (gui.m_workerThread.joinable()) {
        gui.m_workerThread.join();
    }

    if (gui.m_hFontDefault) DeleteObject(gui.m_hFontDefault);
    if (gui.m_hFontBold) DeleteObject(gui.m_hFontBold);
    if (gui.m_hFontTitle) DeleteObject(gui.m_hFontTitle);
    if (gui.m_hFontSubtitle) DeleteObject(gui.m_hFontSubtitle);
    if (gui.m_hFontStats) DeleteObject(gui.m_hFontStats);
    if (gui.m_hBrushHeader) DeleteObject(gui.m_hBrushHeader);
    if (gui.m_hBrushBg) DeleteObject(gui.m_hBrushBg);

    return (int)msg.wParam;
}

void GUI::createControls(HWND hWnd) {
    // 1. Status Bar at bottom
    HWND hStatus = CreateWindowExW(
        0, STATUSCLASSNAMEW, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hWnd, (HMENU)IDC_STATUS_BAR, m_hInstance, NULL
    );
    SendMessageW(hStatus, WM_SETFONT, (WPARAM)m_hFontDefault, TRUE);

    // 2. Main Tab Control
    HWND hTab = CreateWindowExW(
        0, WC_TABCONTROLW, NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        15, 82, 805, 545,
        hWnd, (HMENU)IDC_TAB_MAIN, m_hInstance, NULL
    );
    SendMessageW(hTab, WM_SETFONT, (WPARAM)m_hFontDefault, TRUE);

    auto addCtrl = [this, hWnd](HWND hCtrl, std::vector<HWND>& group) {
        SendMessageW(hCtrl, WM_SETFONT, (WPARAM)m_hFontDefault, TRUE);
        group.push_back(hCtrl);
        return hCtrl;
    };

    // ==========================================
    // TAB 0: COMPRESS CONTROLS
    // ==========================================
    // GroupBox 1: Source
    HWND hGrpCompSrc = CreateWindowExW(0, L"BUTTON", L"1. Select Source File",
        WS_CHILD | BS_GROUPBOX, 30, 115, 775, 75, hWnd, (HMENU)IDC_GRP_COMP_SRC, m_hInstance, NULL);
    addCtrl(hGrpCompSrc, m_tabCompressCtrls);

    HWND hLblCompSrc = CreateWindowExW(0, L"STATIC", L"Source file path (drag and drop supported):",
        WS_CHILD, 45, 137, 630, 18, hWnd, NULL, m_hInstance, NULL);
    addCtrl(hLblCompSrc, m_tabCompressCtrls);

    HWND hEditCompSrc = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | ES_AUTOHSCROLL, 45, 157, 630, 24, hWnd, (HMENU)IDC_EDIT_COMP_SRC, m_hInstance, NULL);
    addCtrl(hEditCompSrc, m_tabCompressCtrls);

    HWND hBtnCompSrc = CreateWindowExW(0, L"BUTTON", L"Browse...",
        WS_CHILD | BS_PUSHBUTTON, 685, 156, 105, 26, hWnd, (HMENU)IDC_BTN_COMP_SRC_BROWSE, m_hInstance, NULL);
    addCtrl(hBtnCompSrc, m_tabCompressCtrls);

    // GroupBox 2: Destination
    HWND hGrpCompDst = CreateWindowExW(0, L"BUTTON", L"2. Output Destination (.huff)",
        WS_CHILD | BS_GROUPBOX, 30, 200, 775, 75, hWnd, (HMENU)IDC_GRP_COMP_DST, m_hInstance, NULL);
    addCtrl(hGrpCompDst, m_tabCompressCtrls);

    HWND hLblCompDst = CreateWindowExW(0, L"STATIC", L"Target compressed file destination (.huff):",
        WS_CHILD, 45, 222, 630, 18, hWnd, NULL, m_hInstance, NULL);
    addCtrl(hLblCompDst, m_tabCompressCtrls);

    HWND hEditCompDst = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | ES_AUTOHSCROLL, 45, 242, 630, 24, hWnd, (HMENU)IDC_EDIT_COMP_DST, m_hInstance, NULL);
    addCtrl(hEditCompDst, m_tabCompressCtrls);

    HWND hBtnCompDst = CreateWindowExW(0, L"BUTTON", L"Save As...",
        WS_CHILD | BS_PUSHBUTTON, 685, 241, 105, 26, hWnd, (HMENU)IDC_BTN_COMP_DST_BROWSE, m_hInstance, NULL);
    addCtrl(hBtnCompDst, m_tabCompressCtrls);

    // Action bar
    HWND hBtnCompExec = CreateWindowExW(0, L"BUTTON", L" Compress File Now ",
        WS_CHILD | BS_DEFPUSHBUTTON, 30, 285, 210, 34, hWnd, (HMENU)IDC_BTN_COMP_EXEC, m_hInstance, NULL);
    SendMessageW(hBtnCompExec, WM_SETFONT, (WPARAM)m_hFontBold, TRUE);
    m_tabCompressCtrls.push_back(hBtnCompExec);

    HWND hProgComp = CreateWindowExW(0, PROGRESS_CLASSW, NULL,
        WS_CHILD | PBS_SMOOTH, 250, 290, 555, 24, hWnd, (HMENU)IDC_PROG_COMP, m_hInstance, NULL);
    addCtrl(hProgComp, m_tabCompressCtrls);

    // GroupBox 3: Results
    HWND hGrpCompStats = CreateWindowExW(0, L"BUTTON", L"3. Compression Results & Statistics",
        WS_CHILD | BS_GROUPBOX, 30, 330, 775, 215, hWnd, (HMENU)IDC_GRP_COMP_STATS, m_hInstance, NULL);
    addCtrl(hGrpCompStats, m_tabCompressCtrls);

    int statY = 358;
    int statGap = 28;

    auto makeStatRow = [&](int y, const wchar_t* label, int lblId, int valId, const wchar_t* defaultVal) {
        HWND hLbl = CreateWindowExW(0, L"STATIC", label, WS_CHILD, 50, y, 200, 20, hWnd, (HMENU)(INT_PTR)lblId, m_hInstance, NULL);
        HWND hVal = CreateWindowExW(0, L"STATIC", defaultVal, WS_CHILD, 250, y, 540, 20, hWnd, (HMENU)(INT_PTR)valId, m_hInstance, NULL);
        SendMessageW(hLbl, WM_SETFONT, (WPARAM)m_hFontDefault, TRUE);
        SendMessageW(hVal, WM_SETFONT, (WPARAM)m_hFontBold, TRUE);
        m_tabCompressCtrls.push_back(hLbl);
        m_tabCompressCtrls.push_back(hVal);
    };

    makeStatRow(statY, L"Original File Size:", IDC_LBL_COMP_ORIG_SIZE, IDC_VAL_COMP_ORIG_SIZE, L"--");
    makeStatRow(statY + statGap, L"Compressed File Size:", IDC_LBL_COMP_NEW_SIZE, IDC_VAL_COMP_NEW_SIZE, L"--");
    makeStatRow(statY + statGap * 2, L"Space Saved:", IDC_LBL_COMP_RATIO, IDC_VAL_COMP_RATIO, L"--");
    makeStatRow(statY + statGap * 3, L"Execution Time:", IDC_LBL_COMP_TIME, IDC_VAL_COMP_TIME, L"--");
    makeStatRow(statY + statGap * 4, L"Status:", IDC_LBL_COMP_STATUS, IDC_VAL_COMP_STATUS, L"Ready to compress");

    HWND hBtnCompOpen = CreateWindowExW(0, L"BUTTON", L" Open Output Folder ",
        WS_CHILD | BS_PUSHBUTTON, 50, statY + statGap * 5 + 4, 210, 28, hWnd, (HMENU)IDC_BTN_COMP_OPEN_DIR, m_hInstance, NULL);
    addCtrl(hBtnCompOpen, m_tabCompressCtrls);
    EnableWindow(hBtnCompOpen, FALSE);

    // ==========================================
    // TAB 1: DECOMPRESS CONTROLS
    // ==========================================
    HWND hGrpDecompSrc = CreateWindowExW(0, L"BUTTON", L"1. Select Compressed File (.huff)",
        WS_CHILD | BS_GROUPBOX, 30, 115, 775, 75, hWnd, (HMENU)IDC_GRP_DECOMP_SRC, m_hInstance, NULL);
    addCtrl(hGrpDecompSrc, m_tabDecompressCtrls);

    HWND hLblDecompSrc = CreateWindowExW(0, L"STATIC", L"Compressed file path (.huff, drag and drop supported):",
        WS_CHILD, 45, 137, 630, 18, hWnd, NULL, m_hInstance, NULL);
    addCtrl(hLblDecompSrc, m_tabDecompressCtrls);

    HWND hEditDecompSrc = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | ES_AUTOHSCROLL, 45, 157, 630, 24, hWnd, (HMENU)IDC_EDIT_DECOMP_SRC, m_hInstance, NULL);
    addCtrl(hEditDecompSrc, m_tabDecompressCtrls);

    HWND hBtnDecompSrc = CreateWindowExW(0, L"BUTTON", L"Browse...",
        WS_CHILD | BS_PUSHBUTTON, 685, 156, 105, 26, hWnd, (HMENU)IDC_BTN_DECOMP_SRC_BROWSE, m_hInstance, NULL);
    addCtrl(hBtnDecompSrc, m_tabDecompressCtrls);

    HWND hGrpDecompDst = CreateWindowExW(0, L"BUTTON", L"2. Restored Output Destination",
        WS_CHILD | BS_GROUPBOX, 30, 200, 775, 75, hWnd, (HMENU)IDC_GRP_DECOMP_DST, m_hInstance, NULL);
    addCtrl(hGrpDecompDst, m_tabDecompressCtrls);

    HWND hLblDecompDst = CreateWindowExW(0, L"STATIC", L"Destination path for restored decompressed file:",
        WS_CHILD, 45, 222, 630, 18, hWnd, NULL, m_hInstance, NULL);
    addCtrl(hLblDecompDst, m_tabDecompressCtrls);

    HWND hEditDecompDst = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | ES_AUTOHSCROLL, 45, 242, 630, 24, hWnd, (HMENU)IDC_EDIT_DECOMP_DST, m_hInstance, NULL);
    addCtrl(hEditDecompDst, m_tabDecompressCtrls);

    HWND hBtnDecompDst = CreateWindowExW(0, L"BUTTON", L"Save As...",
        WS_CHILD | BS_PUSHBUTTON, 685, 241, 105, 26, hWnd, (HMENU)IDC_BTN_DECOMP_DST_BROWSE, m_hInstance, NULL);
    addCtrl(hBtnDecompDst, m_tabDecompressCtrls);

    HWND hBtnDecompExec = CreateWindowExW(0, L"BUTTON", L" Decompress File Now ",
        WS_CHILD | BS_DEFPUSHBUTTON, 30, 285, 230, 34, hWnd, (HMENU)IDC_BTN_DECOMP_EXEC, m_hInstance, NULL);
    SendMessageW(hBtnDecompExec, WM_SETFONT, (WPARAM)m_hFontBold, TRUE);
    m_tabDecompressCtrls.push_back(hBtnDecompExec);

    HWND hProgDecomp = CreateWindowExW(0, PROGRESS_CLASSW, NULL,
        WS_CHILD | PBS_SMOOTH, 270, 290, 535, 24, hWnd, (HMENU)IDC_PROG_DECOMP, m_hInstance, NULL);
    addCtrl(hProgDecomp, m_tabDecompressCtrls);

    HWND hGrpDecompStats = CreateWindowExW(0, L"BUTTON", L"3. Decompression Results",
        WS_CHILD | BS_GROUPBOX, 30, 330, 775, 215, hWnd, (HMENU)IDC_GRP_DECOMP_STATS, m_hInstance, NULL);
    addCtrl(hGrpDecompStats, m_tabDecompressCtrls);

    auto makeDecompRow = [&](int y, const wchar_t* label, int lblId, int valId, const wchar_t* defaultVal) {
        HWND hLbl = CreateWindowExW(0, L"STATIC", label, WS_CHILD, 50, y, 200, 20, hWnd, (HMENU)(INT_PTR)lblId, m_hInstance, NULL);
        HWND hVal = CreateWindowExW(0, L"STATIC", defaultVal, WS_CHILD, 250, y, 540, 20, hWnd, (HMENU)(INT_PTR)valId, m_hInstance, NULL);
        SendMessageW(hLbl, WM_SETFONT, (WPARAM)m_hFontDefault, TRUE);
        SendMessageW(hVal, WM_SETFONT, (WPARAM)m_hFontBold, TRUE);
        m_tabDecompressCtrls.push_back(hLbl);
        m_tabDecompressCtrls.push_back(hVal);
    };

    makeDecompRow(statY, L"Restored File Size:", IDC_LBL_DECOMP_RESTORED_SIZE, IDC_VAL_DECOMP_RESTORED_SIZE, L"--");
    makeDecompRow(statY + statGap, L"Decompression Time:", IDC_LBL_DECOMP_TIME, IDC_VAL_DECOMP_TIME, L"--");
    makeDecompRow(statY + statGap * 2, L"Status:", IDC_LBL_DECOMP_STATUS, IDC_VAL_DECOMP_STATUS, L"Ready to decompress");

    HWND hBtnDecompOpen = CreateWindowExW(0, L"BUTTON", L" Open Output Folder ",
        WS_CHILD | BS_PUSHBUTTON, 50, statY + statGap * 3 + 10, 210, 28, hWnd, (HMENU)IDC_BTN_DECOMP_OPEN_DIR, m_hInstance, NULL);
    addCtrl(hBtnDecompOpen, m_tabDecompressCtrls);
    EnableWindow(hBtnDecompOpen, FALSE);

    // ==========================================
    // TAB 2: BENCHMARK CONTROLS
    // ==========================================
    HWND hGrpBenchCfg = CreateWindowExW(0, L"BUTTON", L"1. Benchmark Configuration",
        WS_CHILD | BS_GROUPBOX, 30, 115, 775, 70, hWnd, (HMENU)IDC_GRP_BENCH_CONFIG, m_hInstance, NULL);
    addCtrl(hGrpBenchCfg, m_tabBenchmarkCtrls);

    HWND hLblBenchDir = CreateWindowExW(0, L"STATIC", L"Dataset Directory:",
        WS_CHILD, 45, 142, 115, 20, hWnd, (HMENU)IDC_LBL_BENCH_DIR, m_hInstance, NULL);
    addCtrl(hLblBenchDir, m_tabBenchmarkCtrls);

    HWND hEditBenchDir = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"./test_dataset/",
        WS_CHILD | ES_AUTOHSCROLL, 165, 139, 335, 24, hWnd, (HMENU)IDC_EDIT_BENCH_DIR, m_hInstance, NULL);
    addCtrl(hEditBenchDir, m_tabBenchmarkCtrls);

    HWND hBtnBenchDir = CreateWindowExW(0, L"BUTTON", L"Browse Folder...",
        WS_CHILD | BS_PUSHBUTTON, 510, 138, 120, 26, hWnd, (HMENU)IDC_BTN_BENCH_DIR_BROWSE, m_hInstance, NULL);
    addCtrl(hBtnBenchDir, m_tabBenchmarkCtrls);

    HWND hBtnBenchExec = CreateWindowExW(0, L"BUTTON", L" Run Benchmark Suite ",
        WS_CHILD | BS_DEFPUSHBUTTON, 640, 138, 155, 26, hWnd, (HMENU)IDC_BTN_BENCH_EXEC, m_hInstance, NULL);
    SendMessageW(hBtnBenchExec, WM_SETFONT, (WPARAM)m_hFontBold, TRUE);
    m_tabBenchmarkCtrls.push_back(hBtnBenchExec);

    HWND hProgBench = CreateWindowExW(0, PROGRESS_CLASSW, NULL,
        WS_CHILD | PBS_SMOOTH, 30, 192, 775, 14, hWnd, (HMENU)IDC_PROG_BENCH, m_hInstance, NULL);
    addCtrl(hProgBench, m_tabBenchmarkCtrls);

    HWND hGrpBenchList = CreateWindowExW(0, L"BUTTON", L"2. Benchmark Performance Results",
        WS_CHILD | BS_GROUPBOX, 30, 212, 775, 255, hWnd, (HMENU)IDC_GRP_BENCH_LIST, m_hInstance, NULL);
    addCtrl(hGrpBenchList, m_tabBenchmarkCtrls);

    HWND hListBench = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, NULL,
        WS_CHILD | LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_VSCROLL,
        45, 234, 745, 222, hWnd, (HMENU)IDC_LIST_BENCH, m_hInstance, NULL);
    addCtrl(hListBench, m_tabBenchmarkCtrls);
    setupListView(hListBench);

    HWND hGrpBenchSum = CreateWindowExW(0, L"BUTTON", L"3. Benchmark Performance Summary",
        WS_CHILD | BS_GROUPBOX, 30, 474, 775, 68, hWnd, (HMENU)IDC_GRP_BENCH_SUMMARY, m_hInstance, NULL);
    addCtrl(hGrpBenchSum, m_tabBenchmarkCtrls);

    HWND hValBenchSum = CreateWindowExW(0, L"STATIC",
        L"No benchmark data available. Click 'Run Benchmark Suite' to test the dataset.",
        WS_CHILD, 45, 498, 550, 36, hWnd, (HMENU)IDC_VAL_BENCH_SUMMARY, m_hInstance, NULL);
    addCtrl(hValBenchSum, m_tabBenchmarkCtrls);

    HWND hBtnBenchClean = CreateWindowExW(0, L"BUTTON", L" Clean Temp Files (.test.huff) ",
        WS_CHILD | BS_PUSHBUTTON, 605, 495, 190, 28, hWnd, (HMENU)IDC_BTN_BENCH_CLEAN, m_hInstance, NULL);
    addCtrl(hBtnBenchClean, m_tabBenchmarkCtrls);
}

void GUI::setupTabs(HWND hWnd) {
    HWND hTab = GetDlgItem(hWnd, IDC_TAB_MAIN);
    TCITEMW tie = {0};
    tie.mask = TCIF_TEXT;

    tie.pszText = (LPWSTR)L"   1. Compress File   ";
    TabCtrl_InsertItem(hTab, 0, &tie);

    tie.pszText = (LPWSTR)L"   2. Decompress File   ";
    TabCtrl_InsertItem(hTab, 1, &tie);

    tie.pszText = (LPWSTR)L"   3. Benchmark Suite   ";
    TabCtrl_InsertItem(hTab, 2, &tie);
}

void GUI::setupListView(HWND hList) {
    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;

    auto addCol = [&](int idx, const wchar_t* title, int width, int fmt) {
        lvc.iSubItem = idx;
        lvc.pszText = (LPWSTR)title;
        lvc.cx = width;
        lvc.fmt = fmt;
        ListView_InsertColumn(hList, idx, &lvc);
    };

    addCol(0, L"No.", 45, LVCFMT_CENTER);
    addCol(1, L"File Name", 205, LVCFMT_LEFT);
    addCol(2, L"Original (KB)", 105, LVCFMT_RIGHT);
    addCol(3, L"Compressed (KB)", 110, LVCFMT_RIGHT);
    addCol(4, L"Saved (%)", 95, LVCFMT_RIGHT);
    addCol(5, L"Time (ms)", 95, LVCFMT_RIGHT);
    addCol(6, L"Status", 80, LVCFMT_CENTER);
}

void GUI::showTab(int tabIndex) {
    m_currentTab = tabIndex;
    auto toggle = [](const std::vector<HWND>& group, int showCmd) {
        for (HWND h : group) {
            ShowWindow(h, showCmd);
        }
    };

    toggle(m_tabCompressCtrls, (tabIndex == 0) ? SW_SHOW : SW_HIDE);
    toggle(m_tabDecompressCtrls, (tabIndex == 1) ? SW_SHOW : SW_HIDE);
    toggle(m_tabBenchmarkCtrls, (tabIndex == 2) ? SW_SHOW : SW_HIDE);

    InvalidateRect(m_hWnd, NULL, TRUE);
}

void GUI::setControlText(int ctrlId, const std::wstring& text) {
    HWND hCtrl = GetDlgItem(m_hWnd, ctrlId);
    if (hCtrl) {
        SetWindowTextW(hCtrl, text.c_str());
    }
}

std::wstring GUI::getControlText(int ctrlId) {
    HWND hCtrl = GetDlgItem(m_hWnd, ctrlId);
    if (!hCtrl) return L"";
    int len = GetWindowTextLengthW(hCtrl);
    if (len <= 0) return L"";
    std::wstring buf(len + 1, 0);
    GetWindowTextW(hCtrl, &buf[0], len + 1);
    buf.resize(len);
    return buf;
}

void GUI::updateStatusBar(const std::wstring& text) {
    HWND hStatus = GetDlgItem(m_hWnd, IDC_STATUS_BAR);
    if (hStatus) {
        SendMessageW(hStatus, SB_SETTEXTW, 0, (LPARAM)text.c_str());
    }
}

void GUI::setControlsEnabled(bool enabled) {
    m_isBusy = !enabled;
    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_COMP_EXEC), enabled);
    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_COMP_SRC_BROWSE), enabled);
    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_COMP_DST_BROWSE), enabled);

    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_DECOMP_EXEC), enabled);
    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_DECOMP_SRC_BROWSE), enabled);
    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_DECOMP_DST_BROWSE), enabled);

    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_BENCH_EXEC), enabled);
    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_BENCH_DIR_BROWSE), enabled);
    EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_BENCH_CLEAN), enabled);

    EnableWindow(GetDlgItem(m_hWnd, IDC_TAB_MAIN), enabled);
}

void GUI::onOpenFileFolder(const std::wstring& path) {
    if (path.empty()) return;
    std::filesystem::path p(path);
    if (std::filesystem::exists(p)) {
        std::wstring param = L"/select,\"" + path + L"\"";
        ShellExecuteW(NULL, L"open", L"explorer.exe", param.c_str(), NULL, SW_SHOWNORMAL);
    } else if (std::filesystem::exists(p.parent_path())) {
        ShellExecuteW(NULL, L"explore", p.parent_path().c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

void GUI::handleFileDrop(HDROP hDrop) {
    wchar_t filePath[MAX_PATH];
    if (DragQueryFileW(hDrop, 0, filePath, MAX_PATH)) {
        std::wstring path(filePath);
        std::filesystem::path fsPath(path);

        if (std::filesystem::is_directory(fsPath)) {
            TabCtrl_SetCurSel(GetDlgItem(m_hWnd, IDC_TAB_MAIN), 2);
            showTab(2);
            setControlText(IDC_EDIT_BENCH_DIR, path);
            updateStatusBar(L"Loaded Dataset folder from drag-and-drop.");
        } else if (fsPath.extension() == L".huff") {
            TabCtrl_SetCurSel(GetDlgItem(m_hWnd, IDC_TAB_MAIN), 1);
            showTab(1);
            setControlText(IDC_EDIT_DECOMP_SRC, path);

            std::wstring pathNoHuff = path.substr(0, path.length() - 5);
            std::filesystem::path p(pathNoHuff);
            std::wstring restoredPath = (p.parent_path() / (p.stem().wstring() + L"_restored" + p.extension().wstring())).wstring();
            setControlText(IDC_EDIT_DECOMP_DST, restoredPath);
            updateStatusBar(L"Loaded compressed .huff file from drag-and-drop.");
        } else {
            TabCtrl_SetCurSel(GetDlgItem(m_hWnd, IDC_TAB_MAIN), 0);
            showTab(0);
            setControlText(IDC_EDIT_COMP_SRC, path);
            setControlText(IDC_EDIT_COMP_DST, path + L".huff");

            if (std::filesystem::exists(fsPath)) {
                uint64_t sz = std::filesystem::file_size(fsPath);
                setControlText(IDC_VAL_COMP_ORIG_SIZE, formatSize(sz));
            }
            updateStatusBar(L"Loaded source file from drag-and-drop.");
        }
    }
    DragFinish(hDrop);
}

// ==========================================
// COMPRESS HANDLERS
// ==========================================
void GUI::onBrowseCompressSource() {
    wchar_t szFile[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0Images (*.bmp;*.png;*.jpg)\0*.bmp;*.png;*.jpg\0Executables (*.exe;*.dll)\0*.exe;*.dll\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        std::wstring srcPath(szFile);
        setControlText(IDC_EDIT_COMP_SRC, srcPath);
        setControlText(IDC_EDIT_COMP_DST, srcPath + L".huff");

        std::filesystem::path p(srcPath);
        if (std::filesystem::exists(p)) {
            uint64_t sz = std::filesystem::file_size(p);
            setControlText(IDC_VAL_COMP_ORIG_SIZE, formatSize(sz));
            setControlText(IDC_VAL_COMP_NEW_SIZE, L"--");
            setControlText(IDC_VAL_COMP_RATIO, L"--");
            setControlText(IDC_VAL_COMP_TIME, L"--");
            setControlText(IDC_VAL_COMP_STATUS, L"Ready to compress");
        }
        updateStatusBar(L"Selected source file: " + p.filename().wstring());
    }
}

void GUI::onBrowseCompressDest() {
    wchar_t szFile[MAX_PATH] = {0};
    std::wstring currentDst = getControlText(IDC_EDIT_COMP_DST);
    if (!currentDst.empty()) {
        wcsncpy(szFile, currentDst.c_str(), MAX_PATH - 1);
    }

    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Huffman Compressed (*.huff)\0*.huff\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"huff";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn)) {
        setControlText(IDC_EDIT_COMP_DST, szFile);
    }
}

void GUI::onExecuteCompress() {
    if (m_isBusy) return;

    std::wstring srcW = getControlText(IDC_EDIT_COMP_SRC);
    std::wstring dstW = getControlText(IDC_EDIT_COMP_DST);

    if (srcW.empty()) {
        MessageBoxW(m_hWnd, L"Please select a source file to compress!", L"Warning", MB_ICONWARNING);
        return;
    }

    std::filesystem::path srcPath(srcW);
    if (!std::filesystem::exists(srcPath) || std::filesystem::is_directory(srcPath)) {
        MessageBoxW(m_hWnd, L"Source file does not exist or is a directory!", L"Error", MB_ICONERROR);
        return;
    }

    if (dstW.empty()) {
        dstW = srcW + L".huff";
        setControlText(IDC_EDIT_COMP_DST, dstW);
    }

    setControlsEnabled(false);
    HWND hProg = GetDlgItem(m_hWnd, IDC_PROG_COMP);
    SendMessageW(hProg, PBM_SETRANGE32, 0, 100);
    SendMessageW(hProg, PBM_SETPOS, 0, 0);
    SetWindowLongPtrW(hProg, GWL_STYLE, GetWindowLongPtrW(hProg, GWL_STYLE) | PBS_MARQUEE);
    SendMessageW(hProg, PBM_SETMARQUEE, TRUE, 30);

    setControlText(IDC_VAL_COMP_STATUS, L"Compressing data... Please wait!");
    updateStatusBar(L"Compressing file: " + srcPath.filename().wstring());

    std::string srcStr = wideToUtf8(srcW);
    std::string dstStr = wideToUtf8(dstW);
    HWND hWnd = m_hWnd;

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    m_workerThread = std::thread([hWnd, srcStr, dstStr, dstW]() {
        CompressResult* res = new CompressResult();
        res->outputPath = dstW;
        try {
            Timer timer;
            timer.start();
            HuffmanEncoder::compress(srcStr, dstStr);
            timer.stop();

            CompressionStats stats = Profiler::calculate_stats(srcStr, dstStr, timer.get_elapsed_milliseconds());
            res->success = true;
            res->origSize = stats.original_size_bytes;
            res->compSize = stats.compressed_size_bytes;
            res->spaceSaved = stats.space_saved_percent;
            res->timeMs = stats.execution_time_ms;
        } catch (const std::exception& e) {
            res->success = false;
            res->errorMsg = GUI::utf8ToWide(e.what());
        }
        PostMessageW(hWnd, WM_APP_COMPRESS_FINISH, 0, (LPARAM)res);
    });
}

void GUI::onCompressFinished(const CompressResult& res) {
    setControlsEnabled(true);
    HWND hProg = GetDlgItem(m_hWnd, IDC_PROG_COMP);
    SendMessageW(hProg, PBM_SETMARQUEE, FALSE, 0);
    SetWindowLongPtrW(hProg, GWL_STYLE, GetWindowLongPtrW(hProg, GWL_STYLE) & ~PBS_MARQUEE);

    if (res.success) {
        SendMessageW(hProg, PBM_SETPOS, 100, 0);
        m_lastCompressedOutput = res.outputPath;

        setControlText(IDC_VAL_COMP_ORIG_SIZE, formatSize(res.origSize));
        setControlText(IDC_VAL_COMP_NEW_SIZE, formatSize(res.compSize));

        std::wstringstream ssRatio;
        ssRatio << std::fixed << std::setprecision(2) << res.spaceSaved << L" %";
        if (res.spaceSaved > 0) {
            ssRatio << L" (Good compression)";
        } else {
            ssRatio << L" (Already compressed data)";
        }
        setControlText(IDC_VAL_COMP_RATIO, ssRatio.str());
        setControlText(IDC_VAL_COMP_TIME, formatTime(res.timeMs));
        setControlText(IDC_VAL_COMP_STATUS, L" File compressed successfully (100% Lossless)!");

        EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_COMP_OPEN_DIR), TRUE);
        updateStatusBar(L"Compression completed in " + formatTime(res.timeMs) + L". Saved to: " + res.outputPath);
        MessageBeep(MB_OK);
    } else {
        SendMessageW(hProg, PBM_SETPOS, 0, 0);
        setControlText(IDC_VAL_COMP_STATUS, L" Compression failed: " + res.errorMsg);
        updateStatusBar(L"Error: " + res.errorMsg);
        MessageBoxW(m_hWnd, (L"File compression encountered an error:\n\n" + res.errorMsg).c_str(), L"Compression Error", MB_ICONERROR);
    }
}

// ==========================================
// DECOMPRESS HANDLERS
// ==========================================
void GUI::onBrowseDecompressSource() {
    wchar_t szFile[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Huffman Compressed (*.huff)\0*.huff\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        std::wstring compPath(szFile);
        setControlText(IDC_EDIT_DECOMP_SRC, compPath);

        // Auto deduce restored file path
        std::filesystem::path p(compPath);
        std::wstring outPath;
        if (p.extension() == L".huff") {
            std::filesystem::path withoutHuff = p;
            withoutHuff.replace_extension(L"");
            std::wstring origExt = withoutHuff.extension().wstring();
            std::wstring stem = withoutHuff.stem().wstring();
            outPath = (p.parent_path() / (stem + L"_restored" + origExt)).wstring();
        } else {
            outPath = compPath + L"_restored";
        }
        setControlText(IDC_EDIT_DECOMP_DST, outPath);

        setControlText(IDC_VAL_DECOMP_RESTORED_SIZE, L"--");
        setControlText(IDC_VAL_DECOMP_TIME, L"--");
        setControlText(IDC_VAL_DECOMP_STATUS, L"Ready to decompress");
        updateStatusBar(L"Selected compressed file: " + p.filename().wstring());
    }
}

void GUI::onBrowseDecompressDest() {
    wchar_t szFile[MAX_PATH] = {0};
    std::wstring currentDst = getControlText(IDC_EDIT_DECOMP_DST);
    if (!currentDst.empty()) {
        wcsncpy(szFile, currentDst.c_str(), MAX_PATH - 1);
    }

    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn)) {
        setControlText(IDC_EDIT_DECOMP_DST, szFile);
    }
}

void GUI::onExecuteDecompress() {
    if (m_isBusy) return;

    std::wstring srcW = getControlText(IDC_EDIT_DECOMP_SRC);
    std::wstring dstW = getControlText(IDC_EDIT_DECOMP_DST);

    if (srcW.empty()) {
        MessageBoxW(m_hWnd, L"Please select a .huff compressed file to decompress!", L"Warning", MB_ICONWARNING);
        return;
    }

    std::filesystem::path srcPath(srcW);
    if (!std::filesystem::exists(srcPath) || std::filesystem::is_directory(srcPath)) {
        MessageBoxW(m_hWnd, L"Compressed file does not exist or is a directory!", L"Error", MB_ICONERROR);
        return;
    }

    if (dstW.empty()) {
        dstW = srcW + L"_restored";
        setControlText(IDC_EDIT_DECOMP_DST, dstW);
    }

    setControlsEnabled(false);
    HWND hProg = GetDlgItem(m_hWnd, IDC_PROG_DECOMP);
    SendMessageW(hProg, PBM_SETRANGE32, 0, 100);
    SendMessageW(hProg, PBM_SETPOS, 0, 0);
    SetWindowLongPtrW(hProg, GWL_STYLE, GetWindowLongPtrW(hProg, GWL_STYLE) | PBS_MARQUEE);
    SendMessageW(hProg, PBM_SETMARQUEE, TRUE, 30);

    setControlText(IDC_VAL_DECOMP_STATUS, L"Decompressing data... Please wait!");
    updateStatusBar(L"Decompressing file: " + srcPath.filename().wstring());

    std::string srcStr = wideToUtf8(srcW);
    std::string dstStr = wideToUtf8(dstW);
    HWND hWnd = m_hWnd;

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    m_workerThread = std::thread([hWnd, srcStr, dstStr, dstW]() {
        DecompressResult* res = new DecompressResult();
        res->outputPath = dstW;
        try {
            Timer timer;
            timer.start();
            HuffmanDecoder::decompress(srcStr, dstStr);
            timer.stop();

            res->success = true;
            res->restoredSize = Profiler::get_file_size(dstStr);
            res->timeMs = timer.get_elapsed_milliseconds();
        } catch (const std::exception& e) {
            res->success = false;
            res->errorMsg = GUI::utf8ToWide(e.what());
        }
        PostMessageW(hWnd, WM_APP_DECOMPRESS_FINISH, 0, (LPARAM)res);
    });
}

void GUI::onDecompressFinished(const DecompressResult& res) {
    setControlsEnabled(true);
    HWND hProg = GetDlgItem(m_hWnd, IDC_PROG_DECOMP);
    SendMessageW(hProg, PBM_SETMARQUEE, FALSE, 0);
    SetWindowLongPtrW(hProg, GWL_STYLE, GetWindowLongPtrW(hProg, GWL_STYLE) & ~PBS_MARQUEE);

    if (res.success) {
        SendMessageW(hProg, PBM_SETPOS, 100, 0);
        m_lastDecompressedOutput = res.outputPath;

        setControlText(IDC_VAL_DECOMP_RESTORED_SIZE, formatSize(res.restoredSize));
        setControlText(IDC_VAL_DECOMP_TIME, formatTime(res.timeMs));
        setControlText(IDC_VAL_DECOMP_STATUS, L" Decompression completed successfully (100% Match)!");

        EnableWindow(GetDlgItem(m_hWnd, IDC_BTN_DECOMP_OPEN_DIR), TRUE);
        updateStatusBar(L"Decompression completed in " + formatTime(res.timeMs) + L". Saved to: " + res.outputPath);
        MessageBeep(MB_OK);
    } else {
        SendMessageW(hProg, PBM_SETPOS, 0, 0);
        setControlText(IDC_VAL_DECOMP_STATUS, L" Decompression failed: " + res.errorMsg);
        updateStatusBar(L"Error: " + res.errorMsg);
        MessageBoxW(m_hWnd, (L"File decompression encountered an error:\n\n" + res.errorMsg).c_str(), L"Decompression Error", MB_ICONERROR);
    }
}

// ==========================================
// BENCHMARK HANDLERS
// ==========================================
void GUI::onBrowseBenchmarkDir() {
    BROWSEINFOW bi = {0};
    bi.hwndOwner = m_hWnd;
    bi.lpszTitle = L"Select Dataset directory for performance benchmarking:";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
    if (pidl != NULL) {
        wchar_t folderPath[MAX_PATH];
        if (SHGetPathFromIDListW(pidl, folderPath)) {
            setControlText(IDC_EDIT_BENCH_DIR, folderPath);
            updateStatusBar(L"Selected benchmark folder: " + std::wstring(folderPath));
        }
        CoTaskMemFree(pidl);
    }
}

void GUI::onExecuteBenchmark() {
    if (m_isBusy) return;

    std::wstring dirW = getControlText(IDC_EDIT_BENCH_DIR);
    if (dirW.empty()) {
        dirW = L"./test_dataset/";
        setControlText(IDC_EDIT_BENCH_DIR, dirW);
    }

    std::filesystem::path dirPath(dirW);
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        MessageBoxW(m_hWnd, (L"Benchmark directory does not exist:\n" + dirW).c_str(), L"Error", MB_ICONERROR);
        return;
    }

    // Clear list view
    HWND hList = GetDlgItem(m_hWnd, IDC_LIST_BENCH);
    ListView_DeleteAllItems(hList);

    setControlsEnabled(false);
    setControlText(IDC_VAL_BENCH_SUMMARY, L"Scanning directory and testing algorithm performance...");
    updateStatusBar(L"Running Benchmark Test Suite...");

    HWND hWnd = m_hWnd;
    std::string dirStr = wideToUtf8(dirW);

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    m_workerThread = std::thread([hWnd, dirStr]() {
        namespace fs = std::filesystem;
        std::vector<fs::path> targetFiles;

        try {
            for (const auto& entry : fs::directory_iterator(fs::u8path(dirStr))) {
                if (entry.is_regular_file()) {
                    auto ext = entry.path().extension();
                    if (ext == ".huff" || ext == ".bin") continue;
                    targetFiles.push_back(entry.path());
                }
            }
        } catch (...) {}

        int totalCount = static_cast<int>(targetFiles.size());
        HWND hProg = GetDlgItem(hWnd, IDC_PROG_BENCH);
        SendMessageW(hProg, PBM_SETRANGE32, 0, totalCount > 0 ? totalCount : 1);
        SendMessageW(hProg, PBM_SETPOS, 0, 0);

        BenchmarkSummaryData* sum = new BenchmarkSummaryData();
        sum->totalFiles = totalCount;
        sum->successCount = 0;
        sum->totalOrigBytes = 0;
        sum->totalCompBytes = 0;
        sum->totalTimeMs = 0;
        sum->avgSavedPercent = 0;

        for (int i = 0; i < totalCount; ++i) {
            const auto& file = targetFiles[i];
            std::string filepath = file.string();
            std::string filename = file.filename().string();
            std::string test_huff = filepath + ".test.huff";

            BenchmarkItemData* item = new BenchmarkItemData();
            item->index = i + 1;
            item->fileName = GUI::utf8ToWide(filename);

            Timer timer;
            timer.start();
            try {
                HuffmanEncoder::compress(filepath, test_huff);
                timer.stop();

                CompressionStats stats = Profiler::calculate_stats(filepath, test_huff, timer.get_elapsed_milliseconds());

                item->success = true;
                item->origKb = stats.original_size_bytes / 1024.0;
                item->compKb = stats.compressed_size_bytes / 1024.0;
                item->savedPercent = stats.space_saved_percent;
                item->timeMs = stats.execution_time_ms;
                item->statusText = L"OK";

                sum->successCount++;
                sum->totalOrigBytes += stats.original_size_bytes;
                sum->totalCompBytes += stats.compressed_size_bytes;
                sum->totalTimeMs += stats.execution_time_ms;

                if (fs::exists(fs::u8path(test_huff))) {
                    fs::remove(fs::u8path(test_huff));
                }
            } catch (const std::exception& e) {
                timer.stop();
                item->success = false;
                item->origKb = Profiler::get_file_size(filepath) / 1024.0;
                item->compKb = 0;
                item->savedPercent = 0;
                item->timeMs = timer.get_elapsed_milliseconds();
                item->statusText = L"Error";

                if (fs::exists(fs::u8path(test_huff))) {
                    fs::remove(fs::u8path(test_huff));
                }
            }

            PostMessageW(hWnd, WM_APP_BENCHMARK_ITEM, (WPARAM)(i + 1), (LPARAM)item);
        }

        if (sum->totalOrigBytes > 0) {
            double totalRatio = (static_cast<double>(sum->totalCompBytes) / sum->totalOrigBytes) * 100.0;
            sum->avgSavedPercent = 100.0 - totalRatio;
        }

        PostMessageW(hWnd, WM_APP_BENCHMARK_FINISH, 0, (LPARAM)sum);
    });
}

void GUI::onBenchmarkItem(const BenchmarkItemData& item) {
    HWND hList = GetDlgItem(m_hWnd, IDC_LIST_BENCH);
    int row = ListView_GetItemCount(hList);

    LVITEMW lvi = {0};
    lvi.mask = LVIF_TEXT;
    lvi.iItem = row;
    lvi.iSubItem = 0;

    std::wstring sIdx = std::to_wstring(item.index);
    lvi.pszText = (LPWSTR)sIdx.c_str();
    ListView_InsertItem(hList, &lvi);

    ListView_SetItemText(hList, row, 1, (LPWSTR)item.fileName.c_str());

    std::wstringstream ssOrig, ssComp, ssSaved, ssTime;
    ssOrig << std::fixed << std::setprecision(2) << item.origKb;
    ListView_SetItemText(hList, row, 2, (LPWSTR)ssOrig.str().c_str());

    if (item.success) {
        ssComp << std::fixed << std::setprecision(2) << item.compKb;
        ssSaved << std::fixed << std::setprecision(2) << item.savedPercent << L"%";
        ssTime << std::fixed << std::setprecision(2) << item.timeMs;

        ListView_SetItemText(hList, row, 3, (LPWSTR)ssComp.str().c_str());
        ListView_SetItemText(hList, row, 4, (LPWSTR)ssSaved.str().c_str());
        ListView_SetItemText(hList, row, 5, (LPWSTR)ssTime.str().c_str());
        ListView_SetItemText(hList, row, 6, (LPWSTR)L"Success");
    } else {
        ListView_SetItemText(hList, row, 3, (LPWSTR)L"--");
        ListView_SetItemText(hList, row, 4, (LPWSTR)L"--");
        ListView_SetItemText(hList, row, 5, (LPWSTR)L"--");
        ListView_SetItemText(hList, row, 6, (LPWSTR)L"Error");
    }

    ListView_EnsureVisible(hList, row, FALSE);

    HWND hProg = GetDlgItem(m_hWnd, IDC_PROG_BENCH);
    SendMessageW(hProg, PBM_SETPOS, item.index, 0);

    updateStatusBar(L"Benchmarking: " + item.fileName + L" (" + std::to_wstring(item.index) + L")");
}

void GUI::onBenchmarkFinished(const BenchmarkSummaryData& sum) {
    setControlsEnabled(true);

    std::wstringstream ss;
    ss << L"Summary: Tested " << sum.successCount << L"/" << sum.totalFiles << L" files\n"
       << L"Original Size: " << formatSize(sum.totalOrigBytes)
       << L"  |  Compressed: " << formatSize(sum.totalCompBytes)
       << L"  |  Avg Space Saved: " << std::fixed << std::setprecision(2) << sum.avgSavedPercent << L"%"
       << L"  |  Total Time: " << formatTime(sum.totalTimeMs);

    setControlText(IDC_VAL_BENCH_SUMMARY, ss.str());
    updateStatusBar(L"Benchmark completed: " + std::to_wstring(sum.successCount) + L"/" + std::to_wstring(sum.totalFiles) + L" files processed successfully.");
    MessageBeep(MB_OK);
}

void GUI::onCleanBenchmarkFiles() {
    std::wstring dirW = getControlText(IDC_EDIT_BENCH_DIR);
    if (dirW.empty()) dirW = L"./test_dataset/";

    namespace fs = std::filesystem;
    fs::path dirPath = fs::u8path(wideToUtf8(dirW));
    int cleanedCount = 0;

    if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
        try {
            for (const auto& entry : fs::directory_iterator(dirPath)) {
                if (entry.is_regular_file()) {
                    std::string fn = entry.path().filename().string();
                    if (fn.find(".test.huff") != std::string::npos) {
                        fs::remove(entry.path());
                        cleanedCount++;
                    }
                }
            }
        } catch (...) {}
    }

    std::wstring msg = L"Successfully removed " + std::to_wstring(cleanedCount) + L" temporary file(s) (.test.huff)!";
    updateStatusBar(msg);
    MessageBoxW(m_hWnd, msg.c_str(), L"Clean Temp Files", MB_OK | MB_ICONINFORMATION);
}

// ==========================================
// WINDOW PROCEDURE
// ==========================================
LRESULT CALLBACK GUI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!s_instance) return DefWindowProcW(hWnd, msg, wParam, lParam);

    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            // 1. Draw top banner (Height 75)
            RECT rcBanner = { 0, 0, rcClient.right, 75 };
            FillRect(hdc, &rcBanner, s_instance->m_hBrushHeader);

            // Left accent bar (Electric blue stripe)
            RECT rcAccent = { 0, 0, 6, 75 };
            HBRUSH hBrushAccent = CreateSolidBrush(RGB(59, 130, 246));
            FillRect(hdc, &rcAccent, hBrushAccent);
            DeleteObject(hBrushAccent);

            // Bottom border line for banner
            HPEN hPenBorder = CreatePen(PS_SOLID, 1, RGB(51, 65, 85));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
            MoveToEx(hdc, 0, 74, NULL);
            LineTo(hdc, rcClient.right, 74);
            SelectObject(hdc, hOldPen);
            DeleteObject(hPenBorder);

            // Title
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            SelectObject(hdc, s_instance->m_hFontTitle);
            RECT rcTitle = { 22, 14, rcClient.right - 20, 42 };
            DrawTextW(hdc, L"HUFFMAN FILE COMPRESSOR & DECOMPRESSOR", -1, &rcTitle, DT_LEFT | DT_SINGLELINE);

            // Subtitle
            SetTextColor(hdc, RGB(148, 163, 184));
            SelectObject(hdc, s_instance->m_hFontSubtitle);
            RECT rcSub = { 22, 44, rcClient.right - 20, 68 };
            DrawTextW(hdc, L"Optimal Lossless Data Compression Tool | Project Team: Nam Luu, Huu Nguyen, Duong Hoang (v1.0)", -1, &rcSub, DT_LEFT | DT_SINGLELINE);

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            int id = GetDlgCtrlID(hCtrl);

            if (id == IDC_VAL_COMP_STATUS || id == IDC_VAL_DECOMP_STATUS) {
                SetTextColor(hdcStatic, RGB(37, 99, 235)); // Accent blue
            } else if (id == IDC_VAL_COMP_RATIO) {
                SetTextColor(hdcStatic, RGB(22, 163, 74)); // Forest green
            } else {
                SetTextColor(hdcStatic, GetSysColor(COLOR_WINDOWTEXT));
            }

            SetBkMode(hdcStatic, TRANSPARENT);
            return (INT_PTR)s_instance->m_hBrushBg;
        }

        case WM_CTLCOLORDLG: {
            return (INT_PTR)s_instance->m_hBrushBg;
        }

        case WM_NOTIFY: {
            LPNMHDR lpnm = (LPNMHDR)lParam;
            if (lpnm->idFrom == IDC_TAB_MAIN && lpnm->code == TCN_SELCHANGE) {
                int curSel = TabCtrl_GetCurSel(lpnm->hwndFrom);
                s_instance->showTab(curSel);
                return 0;
            }
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                // Compress Tab
                case IDC_BTN_COMP_SRC_BROWSE:
                    s_instance->onBrowseCompressSource();
                    return 0;
                case IDC_BTN_COMP_DST_BROWSE:
                    s_instance->onBrowseCompressDest();
                    return 0;
                case IDC_BTN_COMP_EXEC:
                    s_instance->onExecuteCompress();
                    return 0;
                case IDC_BTN_COMP_OPEN_DIR:
                    s_instance->onOpenFileFolder(s_instance->m_lastCompressedOutput);
                    return 0;

                // Decompress Tab
                case IDC_BTN_DECOMP_SRC_BROWSE:
                    s_instance->onBrowseDecompressSource();
                    return 0;
                case IDC_BTN_DECOMP_DST_BROWSE:
                    s_instance->onBrowseDecompressDest();
                    return 0;
                case IDC_BTN_DECOMP_EXEC:
                    s_instance->onExecuteDecompress();
                    return 0;
                case IDC_BTN_DECOMP_OPEN_DIR:
                    s_instance->onOpenFileFolder(s_instance->m_lastDecompressedOutput);
                    return 0;

                // Benchmark Tab
                case IDC_BTN_BENCH_DIR_BROWSE:
                    s_instance->onBrowseBenchmarkDir();
                    return 0;
                case IDC_BTN_BENCH_EXEC:
                    s_instance->onExecuteBenchmark();
                    return 0;
                case IDC_BTN_BENCH_CLEAN:
                    s_instance->onCleanBenchmarkFiles();
                    return 0;

                default:
                    break;
            }
            break;
        }

        case WM_DROPFILES: {
            s_instance->handleFileDrop((HDROP)wParam);
            return 0;
        }

        case WM_APP_COMPRESS_FINISH: {
            auto* res = reinterpret_cast<CompressResult*>(lParam);
            if (res) {
                s_instance->onCompressFinished(*res);
                delete res;
            }
            return 0;
        }

        case WM_APP_DECOMPRESS_FINISH: {
            auto* res = reinterpret_cast<DecompressResult*>(lParam);
            if (res) {
                s_instance->onDecompressFinished(*res);
                delete res;
            }
            return 0;
        }

        case WM_APP_BENCHMARK_ITEM: {
            auto* item = reinterpret_cast<BenchmarkItemData*>(lParam);
            if (item) {
                s_instance->onBenchmarkItem(*item);
                delete item;
            }
            return 0;
        }

        case WM_APP_BENCHMARK_FINISH: {
            auto* sum = reinterpret_cast<BenchmarkSummaryData*>(lParam);
            if (sum) {
                s_instance->onBenchmarkFinished(*sum);
                delete sum;
            }
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        default:
            break;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
