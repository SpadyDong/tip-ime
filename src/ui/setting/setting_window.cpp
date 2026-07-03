#ifdef _WIN32

#include <windows.h>
#include <commctrl.h>

#include <algorithm>
#include <string>
#include <vector>

#include "engine/config/config_manager.h"
#include "ui/candidate/candidate_window.h"
#include "utils/string_utils.h"

namespace tip {

namespace {

constexpr wchar_t kSettingWindowClassName[] = L"TIPSettingWindow";
constexpr wchar_t kConfigFileName[] = L"config\\default.ini";
constexpr wchar_t kSkinFileName[] = L"skins\\default.json";
constexpr int kMaxCornerRadius = 20;
constexpr int kPreviewWidth = 320;
constexpr int kPreviewHeight = 80;

constexpr int IDC_TRACKBAR_RADIUS = 1001;
constexpr int IDC_LABEL_RADIUS = 1002;
constexpr int IDC_VALUE_RADIUS = 1003;
constexpr int IDC_PREVIEW = 1004;
constexpr int IDC_BUTTON_SAVE = 1005;

struct SettingWindowState {
    HWND hwnd = nullptr;
    HWND trackbar = nullptr;
    HWND valueLabel = nullptr;
    HWND preview = nullptr;
    std::wstring configPath;
    CandidateWindowStyle style;
    int cornerRadius = 8;
    std::vector<CandidateItem> sampleCandidates;
};

std::wstring GetExecutableDirectory() {
    wchar_t path[MAX_PATH] = {};
    DWORD len = ::GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len == 0) {
        return std::wstring();
    }
    std::wstring modulePath(path, len);
    size_t pos = modulePath.find_last_of(L"\\/");
    if (pos == std::wstring::npos) {
        return std::wstring();
    }
    return modulePath.substr(0, pos);
}

std::wstring FindDataDirectory() {
    std::wstring exeDir = GetExecutableDirectory();
    if (exeDir.empty()) {
        return std::wstring();
    }
    std::vector<std::wstring> candidates = {
        exeDir + L"\\data",
        exeDir + L"\\..\\data",
        exeDir + L"\\..\\..\\..\\data",
    };
    for (const auto& dir : candidates) {
        DWORD attrs = ::GetFileAttributesW(dir.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return dir;
        }
    }
    return exeDir + L"\\data";
}

void UpdatePreview(HWND hwnd) {
    auto* state = reinterpret_cast<SettingWindowState*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!state || !state->preview) {
        return;
    }
    state->cornerRadius = static_cast<int>(::SendMessageW(state->trackbar, TBM_GETPOS, 0, 0));
    ::SetWindowTextW(state->valueLabel, (std::to_wstring(state->cornerRadius) + L" px").c_str());
    ::InvalidateRect(state->preview, nullptr, FALSE);
}

void SaveSettings(SettingWindowState* state) {
    if (state->configPath.empty()) {
        return;
    }
    ConfigManager::Instance().Load(state->configPath);
    ConfigManager::Instance().GetConfig().cornerRadius = state->cornerRadius;
    ConfigManager::Instance().Save(state->configPath);
    ::MessageBoxW(state->hwnd, L"设置已保存", L"TIP Setting", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK SettingWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<SettingWindowState*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            state = reinterpret_cast<SettingWindowState*>(createStruct->lpCreateParams);
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            state->hwnd = hwnd;

            HINSTANCE hInstance = ::GetModuleHandle(nullptr);

            ::CreateWindowW(L"STATIC", L"候选框圆角大小：",
                            WS_CHILD | WS_VISIBLE | SS_LEFT,
                            20, 20, 120, 20, hwnd, reinterpret_cast<HMENU>(IDC_LABEL_RADIUS),
                            hInstance, nullptr);

            state->trackbar = ::CreateWindowW(TRACKBAR_CLASS, L"",
                                              WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
                                              150, 20, 200, 30, hwnd,
                                              reinterpret_cast<HMENU>(IDC_TRACKBAR_RADIUS),
                                              hInstance, nullptr);
            ::SendMessageW(state->trackbar, TBM_SETRANGE, TRUE, MAKELPARAM(0, kMaxCornerRadius));
            ::SendMessageW(state->trackbar, TBM_SETPOS, TRUE, state->cornerRadius);
            ::SendMessageW(state->trackbar, TBM_SETTICFREQ, 1, 0);

            state->valueLabel = ::CreateWindowW(L"STATIC",
                                                (std::to_wstring(state->cornerRadius) + L" px").c_str(),
                                                WS_CHILD | WS_VISIBLE | SS_LEFT,
                                                360, 20, 60, 20, hwnd,
                                                reinterpret_cast<HMENU>(IDC_VALUE_RADIUS),
                                                hInstance, nullptr);

            ::CreateWindowW(L"STATIC", L"预览：",
                            WS_CHILD | WS_VISIBLE | SS_LEFT,
                            20, 70, 60, 20, hwnd, nullptr, hInstance, nullptr);

            state->preview = ::CreateWindowW(L"STATIC", L"",
                                            WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | WS_BORDER,
                                            20, 100, kPreviewWidth, kPreviewHeight,
                                            hwnd, reinterpret_cast<HMENU>(IDC_PREVIEW),
                                            hInstance, nullptr);

            ::CreateWindowW(L"BUTTON", L"保存",
                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                            20, 200, 80, 28, hwnd,
                            reinterpret_cast<HMENU>(IDC_BUTTON_SAVE),
                            hInstance, nullptr);

            UpdatePreview(hwnd);
            return 0;
        }
        case WM_HSCROLL: {
            if (state && (reinterpret_cast<HWND>(lParam) == state->trackbar)) {
                UpdatePreview(hwnd);
            }
            return 0;
        }
        case WM_DRAWITEM: {
            auto* drawItem = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
            if (state && drawItem->CtlID == IDC_PREVIEW) {
                RECT rect = drawItem->rcItem;
                ::FillRect(drawItem->hDC, &rect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
                DrawCandidateWindow(drawItem->hDC, rect, state->style, state->cornerRadius,
                                    state->sampleCandidates, 0);
            }
            return TRUE;
        }
        case WM_COMMAND: {
            if (state && LOWORD(wParam) == IDC_BUTTON_SAVE) {
                SaveSettings(state);
            }
            return 0;
        }
        case WM_CLOSE:
            ::DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

} // namespace

} // namespace tip

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    using namespace tip;

    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES;
    ::InitCommonControlsEx(&icc);

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = SettingWndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = kSettingWindowClassName;
    ::RegisterClassEx(&wcex);

    SettingWindowState state;
    std::wstring dataDir = FindDataDirectory();
    if (!dataDir.empty()) {
        state.configPath = dataDir + L"\\" + kConfigFileName;
        LoadCandidateWindowStyleFromFile(dataDir + L"\\" + kSkinFileName, state.style);
    }

    ConfigManager::Instance().Load(state.configPath);
    state.cornerRadius = ConfigManager::Instance().GetConfig().cornerRadius;
    state.cornerRadius = std::max(0, std::min(state.cornerRadius, kMaxCornerRadius));

    state.sampleCandidates = {
        { L"中国", L"zhongguo", 0 },
        { L"北京", L"beijing", 1 },
        { L"上海", L"shanghai", 2 },
        { L"中华", L"zhonghua", 3 },
        { L"中央", L"zhongyang", 4 },
    };

    HWND hwnd = ::CreateWindowEx(0, kSettingWindowClassName, L"TIP输入法设置",
                                 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                 CW_USEDEFAULT, CW_USEDEFAULT, 420, 320,
                                 nullptr, nullptr, hInstance, &state);
    ::ShowWindow(hwnd, nCmdShow);
    ::UpdateWindow(hwnd);

    MSG msg;
    while (::GetMessage(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

#else

int main() {
    return 0;
}

#endif // _WIN32
