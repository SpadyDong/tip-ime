#ifdef _WIN32

#include <windows.h>
#include <commdlg.h>

#include <string>

#include "engine/config/config_manager.h"
#include "engine/dictionary/dictionary.h"

namespace {

constexpr int kWindowWidth = 360;
constexpr int kWindowHeight = 340;

constexpr int kLabelWidth = 140;
constexpr int kControlHeight = 22;
constexpr int kMargin = 20;
constexpr int kSpacing = 12;

constexpr int kEditWidth = 160;
constexpr int kCheckSize = 16;
constexpr int kButtonWidth = 80;
constexpr int kButtonHeight = 26;
constexpr int kWideButtonWidth = 120;

HWND hwndCandidateCount = nullptr;
HWND hwndEnableFuzzy = nullptr;
HWND hwndEnableJianpin = nullptr;
HWND hwndSwitchKey = nullptr;
HWND hwndHighDpi = nullptr;

const wchar_t* ConfigPath() {
    return L"data/config/default.ini";
}

const wchar_t* UserDictPath() {
    return L"data/dictionary/user_dict.txt";
}

std::wstring GetWindowTextString(HWND hwnd) {
    int len = GetWindowTextLengthW(hwnd);
    if (len <= 0) {
        return L"";
    }
    std::wstring text;
    text.resize(static_cast<size_t>(len) + 1, L'\0');
    GetWindowTextW(hwnd, &text[0], len + 1);
    text.resize(static_cast<size_t>(len));
    return text;
}

void LoadControlsFromConfig() {
    auto& config = tip::ConfigManager::Instance().GetConfig();

    SetWindowTextW(hwndCandidateCount, std::to_wstring(config.candidateCount).c_str());
    SendMessageW(hwndEnableFuzzy, BM_SETCHECK, config.enableFuzzySound ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hwndEnableJianpin, BM_SETCHECK, config.enableJianpin ? BST_CHECKED : BST_UNCHECKED, 0);
    SetWindowTextW(hwndSwitchKey, config.switchLanguageKey.c_str());
    SendMessageW(hwndHighDpi, BM_SETCHECK, config.highDpi ? BST_CHECKED : BST_UNCHECKED, 0);
}

void SaveConfigFromControls() {
    auto& config = tip::ConfigManager::Instance().GetConfig();

    std::wstring countText = GetWindowTextString(hwndCandidateCount);
    try {
        int count = std::stoi(countText);
        if (count < 1) count = 1;
        if (count > 9) count = 9;
        config.candidateCount = count;
    } catch (...) {
        // Keep existing value if parsing fails.
    }

    config.enableFuzzySound = (SendMessageW(hwndEnableFuzzy, BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.enableJianpin = (SendMessageW(hwndEnableJianpin, BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.switchLanguageKey = GetWindowTextString(hwndSwitchKey);
    config.highDpi = (SendMessageW(hwndHighDpi, BM_GETCHECK, 0, 0) == BST_CHECKED);

    tip::ConfigManager::Instance().Save(ConfigPath());
}

std::wstring ChooseFileForImport(HWND hwnd) {
    wchar_t fileName[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"词库文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameW(&ofn)) {
        return fileName;
    }
    return L"";
}

std::wstring ChooseFileForExport(HWND hwnd) {
    wchar_t fileName[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"词库文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"txt";
    if (GetSaveFileNameW(&ofn)) {
        return fileName;
    }
    return L"";
}

void ImportUserPhrases(HWND hwnd) {
    std::wstring path = ChooseFileForImport(hwnd);
    if (path.empty()) {
        return;
    }

    tip::Dictionary userDict;
    userDict.LoadFromFile(UserDictPath());
    if (userDict.ImportFromFile(path)) {
        if (userDict.SaveToFile(UserDictPath())) {
            MessageBoxW(hwnd, L"自定义短语导入成功", L"TIP Setting", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxW(hwnd, L"导入成功但保存用户词库失败", L"TIP Setting", MB_OK | MB_ICONWARNING);
        }
    } else {
        MessageBoxW(hwnd, L"自定义短语导入失败", L"TIP Setting", MB_OK | MB_ICONERROR);
    }
}

void ExportUserPhrases(HWND hwnd) {
    std::wstring path = ChooseFileForExport(hwnd);
    if (path.empty()) {
        return;
    }

    tip::Dictionary userDict;
    userDict.LoadFromFile(UserDictPath());
    if (userDict.SaveToFile(path)) {
        MessageBoxW(hwnd, L"自定义短语导出成功", L"TIP Setting", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hwnd, L"自定义短语导出失败", L"TIP Setting", MB_OK | MB_ICONERROR);
    }
}

void CreateSettingControls(HWND hwnd) {
    HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hwnd, GWLP_HINSTANCE));
    int y = kMargin;

    CreateWindowExW(0, L"STATIC", L"每页候选词数量 (1-9):",
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    kMargin, y, kLabelWidth, kControlHeight,
                    hwnd, nullptr, hInstance, nullptr);
    hwndCandidateCount = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                         WS_VISIBLE | WS_CHILD | ES_NUMBER,
                                         kMargin + kLabelWidth + 8, y, 60, kControlHeight,
                                         hwnd, reinterpret_cast<HMENU>(1), hInstance, nullptr);
    y += kControlHeight + kSpacing;

    hwndEnableFuzzy = CreateWindowExW(0, L"BUTTON", L"启用模糊音 (z/zh, c/ch, s/sh 等)",
                                      WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                      kMargin, y, kEditWidth + kLabelWidth, kControlHeight,
                                      hwnd, reinterpret_cast<HMENU>(2), hInstance, nullptr);
    y += kControlHeight + kSpacing;

    hwndEnableJianpin = CreateWindowExW(0, L"BUTTON", L"启用简拼",
                                        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                        kMargin, y, kEditWidth + kLabelWidth, kControlHeight,
                                        hwnd, reinterpret_cast<HMENU>(3), hInstance, nullptr);
    y += kControlHeight + kSpacing;

    CreateWindowExW(0, L"STATIC", L"中英文切换键:",
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    kMargin, y, kLabelWidth, kControlHeight,
                    hwnd, nullptr, hInstance, nullptr);
    hwndSwitchKey = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                    WS_VISIBLE | WS_CHILD,
                                    kMargin + kLabelWidth + 8, y, kEditWidth, kControlHeight,
                                    hwnd, reinterpret_cast<HMENU>(4), hInstance, nullptr);
    y += kControlHeight + kSpacing;

    hwndHighDpi = CreateWindowExW(0, L"BUTTON", L"高 DPI 适配",
                                  WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                  kMargin, y, kEditWidth + kLabelWidth, kControlHeight,
                                  hwnd, reinterpret_cast<HMENU>(5), hInstance, nullptr);
    y += kControlHeight + kSpacing * 2;

    CreateWindowExW(0, L"BUTTON", L"导入自定义短语",
                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    kMargin, y, kWideButtonWidth, kButtonHeight,
                    hwnd, reinterpret_cast<HMENU>(101), hInstance, nullptr);

    CreateWindowExW(0, L"BUTTON", L"导出自定义短语",
                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    kMargin + kWideButtonWidth + kSpacing, y, kWideButtonWidth, kButtonHeight,
                    hwnd, reinterpret_cast<HMENU>(102), hInstance, nullptr);
    y += kButtonHeight + kSpacing * 2;

    int buttonX = kMargin;
    CreateWindowExW(0, L"BUTTON", L"保存",
                    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                    buttonX, y, kButtonWidth, kButtonHeight,
                    hwnd, reinterpret_cast<HMENU>(IDOK), hInstance, nullptr);

    CreateWindowExW(0, L"BUTTON", L"取消",
                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    buttonX + kButtonWidth + kSpacing, y, kButtonWidth, kButtonHeight,
                    hwnd, reinterpret_cast<HMENU>(IDCANCEL), hInstance, nullptr);
}

LRESULT CALLBACK SettingWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            CreateSettingControls(hwnd);
            tip::ConfigManager::Instance().Load(ConfigPath());
            LoadControlsFromConfig();
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == IDOK) {
                SaveConfigFromControls();
                DestroyWindow(hwnd);
                return 0;
            } else if (id == IDCANCEL) {
                DestroyWindow(hwnd);
                return 0;
            } else if (id == 101) {
                ImportUserPhrases(hwnd);
                return 0;
            } else if (id == 102) {
                ExportUserPhrases(hwnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool RegisterSettingWindowClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = SettingWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = L"TIPSettingWindow";
    return RegisterClassExW(&wc) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    if (!RegisterSettingWindowClass(hInstance)) {
        MessageBoxW(nullptr, L"注册设置窗口失败", L"TIP Setting", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_CONTEXTHELP,
        L"TIPSettingWindow",
        L"TIP输入法设置",
        WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        kWindowWidth, kWindowHeight,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"创建设置窗口失败", L"TIP Setting", MB_OK | MB_ICONERROR);
        return 1;
    }

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

#else

int main() {
    return 0;
}

#endif // _WIN32
