#include "candidate_window.h"

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <fstream>
#include <sstream>

#include "utils/string_utils.h"

#ifdef _WIN32

#include <windows.h>

namespace tip {

namespace {

constexpr int kMinCornerRadius = 0;
constexpr int kMaxCornerRadius = 32;
constexpr int kItemPaddingX = 8;
constexpr int kItemPaddingY = 6;
constexpr int kItemSpacing = 4;
constexpr int kWindowMargin = 8;
constexpr int kMinWindowWidth = 60;
constexpr int kMinWindowHeight = 36;

COLORREF ToColorRef(unsigned int rgb) {
    return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

unsigned int ParseHexColor(const std::wstring& hex) {
    if (hex.size() < 6) {
        return 0;
    }
    size_t start = 0;
    if (hex[0] == L'#') {
        start = 1;
    }
    try {
        return static_cast<unsigned int>(std::stoul(hex.substr(start, 6), nullptr, 16));
    } catch (...) {
        return 0;
    }
}

std::wstring FindJsonString(const std::wstring& content, const std::wstring& key) {
    size_t pos = content.find(L"\"" + key + L"\"");
    if (pos == std::wstring::npos) {
        return std::wstring();
    }
    pos = content.find(L':', pos);
    if (pos == std::wstring::npos) {
        return std::wstring();
    }
    pos = content.find(L'"', pos);
    if (pos == std::wstring::npos) {
        return std::wstring();
    }
    size_t end = content.find(L'"', pos + 1);
    if (end == std::wstring::npos) {
        return std::wstring();
    }
    return content.substr(pos + 1, end - pos - 1);
}

int FindJsonInt(const std::wstring& content, const std::wstring& key, int defaultValue) {
    size_t pos = content.find(L"\"" + key + L"\"");
    if (pos == std::wstring::npos) {
        return defaultValue;
    }
    pos = content.find(L':', pos);
    if (pos == std::wstring::npos) {
        return defaultValue;
    }
    ++pos;
    while (pos < content.size() && (content[pos] == L' ' || content[pos] == L'\t')) {
        ++pos;
    }
    size_t end = pos;
    while (end < content.size() && (content[end] == L'-' || std::iswdigit(content[end]))) {
        ++end;
    }
    try {
        return std::stoi(content.substr(pos, end - pos));
    } catch (...) {
        return defaultValue;
    }
}

HFONT CreateStyleFont(const CandidateWindowStyle& style) {
    return ::CreateFont(
        -style.fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, style.fontFamily.c_str());
}

void MeasureCandidates(HDC hdc, const CandidateWindowStyle& style,
                       const std::vector<CandidateItem>& candidates,
                       int& itemWidth, int& itemHeight) {
    HFONT font = CreateStyleFont(style);
    HFONT oldFont = static_cast<HFONT>(::SelectObject(hdc, font));

    int maxTextWidth = 0;
    int textHeight = 0;
    TEXTMETRIC tm;
    ::GetTextMetrics(hdc, &tm);
    textHeight = tm.tmHeight;

    for (const auto& candidate : candidates) {
        std::wstring label = std::to_wstring(candidate.index + 1) + L"." + candidate.text;
        SIZE size;
        ::GetTextExtentPoint32W(hdc, label.c_str(), static_cast<int>(label.size()), &size);
        maxTextWidth = std::max(maxTextWidth, static_cast<int>(size.cx));
    }

    ::SelectObject(hdc, oldFont);
    ::DeleteObject(font);

    itemWidth = maxTextWidth + kItemPaddingX * 2;
    itemHeight = textHeight + kItemPaddingY * 2;
}

} // namespace

bool LoadCandidateWindowStyleFromFile(const std::wstring& filePath, CandidateWindowStyle& style) {
    std::ifstream file(WideToUtf8(filePath));
    if (!file.is_open()) {
        return false;
    }
    std::string contentUtf8((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::wstring content = Utf8ToWide(contentUtf8);

    std::wstring bg = FindJsonString(content, L"background_color");
    if (!bg.empty()) style.backgroundColor = ParseHexColor(bg);
    std::wstring border = FindJsonString(content, L"border_color");
    if (!border.empty()) style.borderColor = ParseHexColor(border);
    std::wstring text = FindJsonString(content, L"text_color");
    if (!text.empty()) style.textColor = ParseHexColor(text);
    std::wstring highlight = FindJsonString(content, L"highlight_color");
    if (!highlight.empty()) style.highlightColor = ParseHexColor(highlight);
    std::wstring selBg = FindJsonString(content, L"selection_background_color");
    if (!selBg.empty()) style.selectionBackgroundColor = ParseHexColor(selBg);
    std::wstring selText = FindJsonString(content, L"selection_text_color");
    if (!selText.empty()) style.selectionTextColor = ParseHexColor(selText);
    std::wstring family = FindJsonString(content, L"font_family");
    if (!family.empty()) style.fontFamily = family;
    style.fontSize = FindJsonInt(content, L"font_size", style.fontSize);
    return true;
}

void DrawCandidateWindow(HDC hdc, const RECT& clientRect, const CandidateWindowStyle& style,
                         int cornerRadius, const std::vector<CandidateItem>& candidates,
                         int selectedIndex) {
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    cornerRadius = std::max(kMinCornerRadius, std::min(cornerRadius, kMaxCornerRadius));

    HBRUSH bgBrush = ::CreateSolidBrush(ToColorRef(style.backgroundColor));
    HPEN borderPen = ::CreatePen(PS_SOLID, 1, ToColorRef(style.borderColor));
    HBRUSH selBrush = ::CreateSolidBrush(ToColorRef(style.selectionBackgroundColor));

    HGDIOBJ oldBrush = ::SelectObject(hdc, bgBrush);
    HGDIOBJ oldPen = ::SelectObject(hdc, borderPen);

    ::RoundRect(hdc, 0, 0, width, height, cornerRadius * 2, cornerRadius * 2);

    ::SelectObject(hdc, oldBrush);
    ::SelectObject(hdc, oldPen);
    ::DeleteObject(bgBrush);
    ::DeleteObject(borderPen);

    if (candidates.empty()) {
        ::DeleteObject(selBrush);
        return;
    }

    HFONT font = CreateStyleFont(style);
    HFONT oldFont = static_cast<HFONT>(::SelectObject(hdc, font));
    ::SetBkMode(hdc, TRANSPARENT);

    int itemWidth = 0;
    int itemHeight = 0;
    MeasureCandidates(hdc, style, candidates, itemWidth, itemHeight);

    int totalItemsWidth = static_cast<int>(candidates.size()) * itemWidth +
                          (static_cast<int>(candidates.size()) - 1) * kItemSpacing;
    int startX = kWindowMargin;
    int startY = kWindowMargin;
    if (totalItemsWidth + kWindowMargin * 2 < width) {
        startX = (width - totalItemsWidth) / 2;
    }
    if (itemHeight + kWindowMargin * 2 < height) {
        startY = (height - itemHeight) / 2;
    }

    for (size_t i = 0; i < candidates.size(); ++i) {
        int x = startX + static_cast<int>(i) * (itemWidth + kItemSpacing);
        RECT itemRect = { x, startY, x + itemWidth, startY + itemHeight };

        if (static_cast<int>(i) == selectedIndex) {
            int saveId = ::SaveDC(hdc);
            ::IntersectClipRect(hdc, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom);
            HGDIOBJ oldSelBrush = ::SelectObject(hdc, selBrush);
            HGDIOBJ oldSelPen = ::SelectObject(hdc, ::GetStockObject(NULL_PEN));
            int itemRadius = std::min(cornerRadius, itemHeight / 2);
            ::RoundRect(hdc, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom,
                        itemRadius * 2, itemRadius * 2);
            ::SelectObject(hdc, oldSelBrush);
            ::SelectObject(hdc, oldSelPen);
            ::RestoreDC(hdc, saveId);
        }

        std::wstring label = std::to_wstring(candidates[i].index + 1) + L"." + candidates[i].text;
        ::SetTextColor(hdc, static_cast<int>(i) == selectedIndex
                                ? ToColorRef(style.selectionTextColor)
                                : ToColorRef(style.textColor));
        ::DrawTextW(hdc, label.c_str(), static_cast<int>(label.size()), &itemRect,
                    DT_SINGLELINE | DT_VCENTER | DT_CENTER);
    }

    ::SelectObject(hdc, oldFont);
    ::DeleteObject(font);
    ::DeleteObject(selBrush);
}

namespace {

void ComputeWindowSize(HDC hdc, const CandidateWindowStyle& style,
                       const std::vector<CandidateItem>& candidates,
                       int& width, int& height) {
    int itemWidth = 0;
    int itemHeight = 0;
    MeasureCandidates(hdc, style, candidates, itemWidth, itemHeight);
    width = kWindowMargin * 2 + static_cast<int>(candidates.size()) * itemWidth +
            (static_cast<int>(candidates.size()) - 1) * kItemSpacing;
    height = kWindowMargin * 2 + itemHeight;
    width = std::max(width, kMinWindowWidth);
    height = std::max(height, kMinWindowHeight);
}

constexpr wchar_t kCandidateWindowClassName[] = L"TIPCandidateWindow";
bool g_classRegistered = false;

LRESULT CALLBACK CandidateWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
        auto* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
        return 0;
    }

    auto* impl = reinterpret_cast<CandidateWindow::Impl*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hwnd, &ps);
            RECT clientRect;
            ::GetClientRect(hwnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;

            HDC memDc = ::CreateCompatibleDC(hdc);
            HBITMAP memBmp = ::CreateCompatibleBitmap(hdc, width, height);
            HBITMAP oldBmp = static_cast<HBITMAP>(::SelectObject(memDc, memBmp));

            DrawCandidateWindow(memDc, clientRect, impl->style, impl->cornerRadius,
                                impl->candidates, static_cast<int>(impl->selectedIndex));

            ::BitBlt(hdc, 0, 0, width, height, memDc, 0, 0, SRCCOPY);
            ::SelectObject(memDc, oldBmp);
            ::DeleteObject(memBmp);
            ::DeleteDC(memDc);
            ::EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            return 0;
        default:
            break;
    }
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

} // namespace

class CandidateWindow::Impl {
public:
    HWND hwnd = nullptr;
    int posX = 0;
    int posY = 0;
    int cornerRadius = 8;
    bool visible = false;
    CandidateWindowStyle style;
    std::vector<CandidateItem> candidates;
    size_t selectedIndex = 0;

    void UpdateRegion() {
        if (!hwnd) {
            return;
        }
        RECT rect;
        ::GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        int radius = std::max(0, std::min(cornerRadius, kMaxCornerRadius));
        HRGN region = ::CreateRoundRectRgn(0, 0, width + 1, height + 1, radius * 2, radius * 2);
        ::SetWindowRgn(hwnd, region, TRUE);
    }

    void RecalculateLayout() {
        if (!hwnd || candidates.empty()) {
            return;
        }
        HDC hdc = ::GetDC(hwnd);
        int width = 0;
        int height = 0;
        ComputeWindowSize(hdc, style, candidates, width, height);
        ::ReleaseDC(hwnd, hdc);
        ::SetWindowPos(hwnd, nullptr, posX, posY, width, height,
                       SWP_NOZORDER | SWP_NOACTIVATE);
        UpdateRegion();
        ::InvalidateRect(hwnd, nullptr, FALSE);
    }
};

CandidateWindow::CandidateWindow()
    : impl_(new Impl()) {
}

CandidateWindow::~CandidateWindow() {
    Destroy();
    delete impl_;
}

bool CandidateWindow::Create() {
    if (impl_->hwnd) {
        return true;
    }

    HINSTANCE hInstance = ::GetModuleHandle(nullptr);
    if (!g_classRegistered) {
        WNDCLASSEX wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.lpfnWndProc = CandidateWndProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        wcex.lpszClassName = kCandidateWindowClassName;
        if (!::RegisterClassEx(&wcex)) {
            return false;
        }
        g_classRegistered = true;
    }

    impl_->hwnd = ::CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST,
        kCandidateWindowClassName, nullptr,
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, kMinWindowWidth, kMinWindowHeight,
        nullptr, nullptr, hInstance, impl_);

    if (!impl_->hwnd) {
        return false;
    }

    impl_->visible = false;
    return true;
}

void CandidateWindow::Destroy() {
    if (impl_->hwnd) {
        ::DestroyWindow(impl_->hwnd);
        impl_->hwnd = nullptr;
    }
    impl_->visible = false;
}

void CandidateWindow::Show() {
    if (impl_->hwnd) {
        ::ShowWindow(impl_->hwnd, SW_SHOWNA);
        impl_->visible = true;
    }
}

void CandidateWindow::Hide() {
    if (impl_->hwnd) {
        ::ShowWindow(impl_->hwnd, SW_HIDE);
        impl_->visible = false;
    }
}

bool CandidateWindow::IsVisible() const {
    return impl_->hwnd && ::IsWindowVisible(impl_->hwnd);
}

void CandidateWindow::SetCornerRadius(int radius) {
    impl_->cornerRadius = radius;
    impl_->UpdateRegion();
    if (impl_->hwnd) {
        ::InvalidateRect(impl_->hwnd, nullptr, FALSE);
    }
}

void CandidateWindow::SetStyle(const CandidateWindowStyle& style) {
    impl_->style = style;
    impl_->RecalculateLayout();
}

void CandidateWindow::UpdateCandidates(const std::vector<CandidateItem>& candidates, size_t selectedIndex) {
    impl_->candidates = candidates;
    impl_->selectedIndex = selectedIndex;
    impl_->RecalculateLayout();
}

void CandidateWindow::MoveTo(int x, int y) {
    impl_->posX = x;
    impl_->posY = y;
    if (impl_->hwnd) {
        ::SetWindowPos(impl_->hwnd, nullptr, x, y, 0, 0,
                       SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

} // namespace tip

#else // Non-Windows stub

namespace tip {

bool LoadCandidateWindowStyleFromFile(const std::wstring&, CandidateWindowStyle&) {
    return false;
}

CandidateWindow::CandidateWindow()
    : impl_(nullptr) {
}

CandidateWindow::~CandidateWindow() = default;

bool CandidateWindow::Create() {
    return true;
}

void CandidateWindow::Destroy() {
}

void CandidateWindow::Show() {
}

void CandidateWindow::Hide() {
}

bool CandidateWindow::IsVisible() const {
    return false;
}

void CandidateWindow::SetCornerRadius(int) {
}

void CandidateWindow::SetStyle(const CandidateWindowStyle&) {
}

void CandidateWindow::UpdateCandidates(const std::vector<CandidateItem>&, size_t) {
}

void CandidateWindow::MoveTo(int, int) {
}

} // namespace tip

#endif // _WIN32
