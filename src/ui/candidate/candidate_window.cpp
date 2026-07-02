#include "candidate_window.h"

#ifdef _WIN32

#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

namespace tip {

namespace {

constexpr int kItemHeight = 36;
constexpr int kItemPaddingY = 6;
constexpr int kItemPaddingX = 10;
constexpr int kIndexTextWidth = 16;
constexpr int kTextHeight = 20;
constexpr int kHighlightBarWidth = 3;
constexpr int kButtonWidth = 24;
constexpr int kLanguageButtonWidth = 32;
constexpr int kCornerRadius = 8;
constexpr int kWindowMargin = 4;

constexpr COLORREF kBackgroundColor = RGB(250, 250, 250);
constexpr COLORREF kBorderColor = RGB(220, 220, 220);
constexpr COLORREF kTextColor = RGB(32, 32, 32);
constexpr COLORREF kIndexColor = RGB(120, 120, 120);
constexpr COLORREF kHighlightColor = RGB(0, 120, 215);
constexpr COLORREF kButtonColor = RGB(100, 100, 100);
constexpr COLORREF kButtonHoverColor = RGB(32, 32, 32);
constexpr COLORREF kButtonDisabledColor = RGB(200, 200, 200);

std::unordered_map<HWND, CandidateWindow::Impl*>& WindowMap() {
    static std::unordered_map<HWND, CandidateWindow::Impl*> s_map;
    return s_map;
}

int GdiplusStartupTokenRefCount() {
    static int s_ref_count = 0;
    return s_ref_count;
}

ULONG_PTR& GdiplusToken() {
    static ULONG_PTR s_token = 0;
    return s_token;
}

void EnsureGdiplusStartup() {
    if (GdiplusStartupTokenRefCount() == 0) {
        Gdiplus::GdiplusStartupInput input;
        input.GdiplusVersion = 1;
        Gdiplus::GdiplusStartup(&GdiplusToken(), &input, nullptr);
    }
    ++GdiplusStartupTokenRefCount();
}

void GdiplusShutdownIfNeeded() {
    --GdiplusStartupTokenRefCount();
    if (GdiplusStartupTokenRefCount() <= 0) {
        Gdiplus::GdiplusShutdown(GdiplusToken());
        GdiplusToken() = 0;
        GdiplusStartupTokenRefCount() = 0;
    }
}

const wchar_t* ClassName() {
    return L"TIPCandidateWindow";
}

HINSTANCE GetCurrentModuleHandle() {
    HINSTANCE hInstance = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        reinterpret_cast<LPCWSTR>(CandidateWndProc),
        &hInstance);
    return hInstance ? hInstance : GetModuleHandleW(nullptr);
}

bool RegisterWindowClass() {
    static bool s_registered = false;
    if (s_registered) {
        return true;
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetCurrentModuleHandle();
    wc.lpszClassName = ClassName();
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);

    if (!RegisterClassExW(&wc)) {
        return false;
    }
    s_registered = true;
    return true;
}

int MeasureCandidateWidth(Gdiplus::Graphics& graphics, const std::wstring& text, Gdiplus::Font* font) {
    Gdiplus::RectF layoutRect(0, 0, 0, 0);
    Gdiplus::RectF boundRect;
    graphics.MeasureString(text.c_str(), static_cast<int>(text.length()), font, layoutRect, &boundRect);
    return static_cast<int>(boundRect.Width) + 1;
}

void DrawRoundedRectangle(Gdiplus::Graphics& graphics, const Gdiplus::Rect& rect, int radius, const Gdiplus::Color& color) {
    Gdiplus::SolidBrush brush(color);
    Gdiplus::GraphicsPath path;
    int diameter = radius * 2;

    path.AddArc(rect.X, rect.Y, diameter, diameter, 180, 90);
    path.AddArc(rect.X + rect.Width - diameter, rect.Y, diameter, diameter, 270, 90);
    path.AddArc(rect.X + rect.Width - diameter, rect.Y + rect.Height - diameter, diameter, diameter, 0, 90);
    path.AddArc(rect.X, rect.Y + rect.Height - diameter, diameter, diameter, 90, 90);
    path.CloseFigure();

    graphics.FillPath(&brush, &path);
}

void DrawArrow(Gdiplus::Graphics& graphics, int x, int y, bool left, const Gdiplus::Color& color) {
    Gdiplus::Pen pen(color, 1.5f);
    Gdiplus::PointF points[3];
    int size = 5;
    if (left) {
        points[0] = Gdiplus::PointF(static_cast<float>(x + size), static_cast<float>(y - size));
        points[1] = Gdiplus::PointF(static_cast<float>(x), static_cast<float>(y));
        points[2] = Gdiplus::PointF(static_cast<float>(x + size), static_cast<float>(y + size));
    } else {
        points[0] = Gdiplus::PointF(static_cast<float>(x), static_cast<float>(y - size));
        points[1] = Gdiplus::PointF(static_cast<float>(x + size), static_cast<float>(y));
        points[2] = Gdiplus::PointF(static_cast<float>(x), static_cast<float>(y + size));
    }
    graphics.DrawLines(&pen, points, 3);
}

void DrawGearIcon(Gdiplus::Graphics& graphics, int cx, int cy, const Gdiplus::Color& color) {
    Gdiplus::Pen pen(color, 1.5f);
    int outerRadius = 7;
    int innerRadius = 4;
    graphics.DrawEllipse(&pen, cx - outerRadius, cy - outerRadius, outerRadius * 2, outerRadius * 2);
    graphics.DrawEllipse(&pen, cx - innerRadius, cy - innerRadius, innerRadius * 2, innerRadius * 2);

    for (int i = 0; i < 8; ++i) {
        float angle = static_cast<float>(i * 3.1415926f / 4.0f);
        float x1 = cx + innerRadius * cosf(angle);
        float y1 = cy + innerRadius * sinf(angle);
        float x2 = cx + outerRadius * cosf(angle);
        float y2 = cy + outerRadius * sinf(angle);
        graphics.DrawLine(&pen, x1, y1, x2, y2);
    }
}

LRESULT CALLBACK CandidateWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto it = WindowMap().find(hwnd);
    if (it == WindowMap().end()) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    CandidateWindow::Impl* impl = it->second;
    switch (msg) {
        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            impl->HandleMouseClick(x, y);
            return 0;
        }
        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            impl->HandleMouseMove(x, y);
            return 0;
        }
        case WM_PAINT: {
            impl->Render();
            return 0;
        }
        case WM_DESTROY:
            WindowMap().erase(hwnd);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

class CandidateWindow::Impl {
public:
    HWND hwnd = nullptr;
    HBITMAP memBitmap = nullptr;
    HDC memDC = nullptr;
    int width = 0;
    int height = 0;
    int posX = 0;
    int posY = 0;
    bool visible = false;
    std::vector<CandidateItem> candidates;
    int hoverButton = -1; // -1 none, 0 prev, 1 next, 2 settings, 3 language
    int selectedIndex = 0;
    int currentPage = 0;
    int totalPages = 1;
    bool chineseMode = true;
    ClickCallback clickCallback;

    std::vector<int> itemWidths;
    int contentWidth = 0;

    Impl() {
        EnsureGdiplusStartup();
    }

    ~Impl() {
        Destroy();
        GdiplusShutdownIfNeeded();
    }

    bool Create() {
        if (hwnd) {
            return true;
        }

        if (!RegisterWindowClass()) {
            return false;
        }

        HINSTANCE hInstance = GetCurrentModuleHandle();
        hwnd = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST,
            ClassName(),
            L"TIP Candidate",
            WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, 1, 1,
            nullptr,
            nullptr,
            hInstance,
            nullptr
        );

        if (!hwnd) {
            return false;
        }

        SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CandidateWndProc));
        WindowMap()[hwnd] = this;

        UpdateLayout();
        return true;
    }

    void Destroy() {
        if (memDC) {
            DeleteDC(memDC);
            memDC = nullptr;
        }
        if (memBitmap) {
            DeleteObject(memBitmap);
            memBitmap = nullptr;
        }
        if (hwnd) {
            DestroyWindow(hwnd);
            WindowMap().erase(hwnd);
            hwnd = nullptr;
        }
        visible = false;
    }

    void Show() {
        if (!hwnd) {
            Create();
        }
        if (hwnd) {
            UpdateLayout();
            SetWindowPos(hwnd, HWND_TOPMOST, posX, posY, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
            visible = true;
        }
    }

    void Hide() {
        if (hwnd) {
            ShowWindow(hwnd, SW_HIDE);
        }
        visible = false;
    }

    void UpdateCandidates(const std::vector<CandidateItem>& items) {
        candidates = items;
        selectedIndex = 0;
        if (visible) {
            UpdateLayout();
            Render();
        }
    }

    void SetPageInfo(int page, int total) {
        currentPage = page;
        totalPages = total;
        if (totalPages < 1) {
            totalPages = 1;
        }
        if (currentPage < 0) {
            currentPage = 0;
        } else if (currentPage >= totalPages) {
            currentPage = totalPages - 1;
        }
    }

    void SetClickCallback(ClickCallback callback) {
        clickCallback = std::move(callback);
    }

    void SetLanguageIndicator(bool mode) {
        if (chineseMode != mode) {
            chineseMode = mode;
            if (visible) {
                Render();
            }
        }
    }

    void MoveTo(int x, int y) {
        posX = x;
        posY = y;
        if (hwnd && visible) {
            SetWindowPos(hwnd, HWND_TOPMOST, posX, posY, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
    }

    void UpdateLayout() {
        HDC screenDC = GetDC(nullptr);
        HDC tempDC = CreateCompatibleDC(screenDC);

        Gdiplus::Graphics graphics(tempDC);
        Gdiplus::Font font(L"Microsoft YaHei UI", 12.0f);

        itemWidths.clear();
        contentWidth = kWindowMargin * 2 + kHighlightBarWidth + kItemPaddingX;

        for (const auto& item : candidates) {
            int textWidth = MeasureCandidateWidth(graphics, item.text, &font);
            int itemWidth = kIndexTextWidth + kItemPaddingX + textWidth + kItemPaddingX;
            itemWidths.push_back(itemWidth);
            contentWidth += itemWidth;
        }

        if (!candidates.empty()) {
            // Buttons: language, prev, next, settings
            contentWidth += kButtonWidth * 3 + kLanguageButtonWidth;
        }

        height = kItemHeight + kWindowMargin * 2;
        width = contentWidth + kWindowMargin * 2;

        DeleteDC(tempDC);
        ReleaseDC(nullptr, screenDC);

        if (hwnd) {
            SetWindowPos(hwnd, HWND_TOPMOST, posX, posY, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
    }

    void Render() {
        if (!hwnd) {
            return;
        }

        HDC screenDC = GetDC(nullptr);
        if (!memDC) {
            memDC = CreateCompatibleDC(screenDC);
        }
        if (memBitmap) {
            DeleteObject(memBitmap);
        }

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        memBitmap = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
        SelectObject(memDC, memBitmap);

        ReleaseDC(nullptr, screenDC);

        Gdiplus::Graphics graphics(memDC);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

        // Clear background
        Gdiplus::Rect fullRect(0, 0, width, height);
        DrawRoundedRectangle(graphics, fullRect, kCornerRadius, Gdiplus::Color(GetRValue(kBackgroundColor), GetGValue(kBackgroundColor), GetBValue(kBackgroundColor)));

        // Draw border
        Gdiplus::Pen borderPen(Gdiplus::Color(GetRValue(kBorderColor), GetGValue(kBorderColor), GetBValue(kBorderColor)), 1.0f);
        Gdiplus::GraphicsPath borderPath;
        int d = kCornerRadius * 2;
        borderPath.AddArc(0, 0, d, d, 180, 90);
        borderPath.AddArc(width - d, 0, d, d, 270, 90);
        borderPath.AddArc(width - d, height - d, d, d, 0, 90);
        borderPath.AddArc(0, height - d, d, d, 90, 90);
        borderPath.CloseFigure();
        graphics.DrawPath(&borderPen, &borderPath);

        // Draw candidates
        int x = kWindowMargin + kHighlightBarWidth + kItemPaddingX;
        int y = kWindowMargin + (kItemHeight - kTextHeight) / 2;

        for (size_t i = 0; i < candidates.size(); ++i) {
            // Highlight bar for first item
            if (i == 0) {
                Gdiplus::SolidBrush highlightBrush(Gdiplus::Color(GetRValue(kHighlightColor), GetGValue(kHighlightColor), GetBValue(kHighlightColor)));
                graphics.FillRectangle(&highlightBrush,
                    kWindowMargin,
                    kWindowMargin + 6,
                    kHighlightBarWidth,
                    kItemHeight - 12);
            }

            // Index number
            std::wstring indexText = std::to_wstring(i + 1) + L".";
            Gdiplus::SolidBrush indexBrush(Gdiplus::Color(GetRValue(kIndexColor), GetGValue(kIndexColor), GetBValue(kIndexColor)));
            Gdiplus::Font indexFont(L"Microsoft YaHei UI", 10.0f);
            Gdiplus::PointF indexPos(static_cast<float>(x), static_cast<float>(y));
            graphics.DrawString(indexText.c_str(), static_cast<int>(indexText.length()), &indexFont, indexPos, &indexBrush);

            // Candidate text
            const auto& item = candidates[i];
            Gdiplus::SolidBrush textBrush(Gdiplus::Color(GetRValue(kTextColor), GetGValue(kTextColor), GetBValue(kTextColor)));
            Gdiplus::Font textFont(L"Microsoft YaHei UI", 12.0f);
            Gdiplus::PointF textPos(static_cast<float>(x + kIndexTextWidth), static_cast<float>(y));
            graphics.DrawString(item.text.c_str(), static_cast<int>(item.text.length()), &textFont, textPos, &textBrush);

            x += itemWidths[i];
        }

        // Draw buttons
        if (!candidates.empty()) {
            int buttonY = kWindowMargin + kItemHeight / 2;

            // Language indicator button
            int langX = width - kWindowMargin - kButtonWidth * 3 - kLanguageButtonWidth;
            Gdiplus::Color langBgColor = (hoverButton == 3)
                ? Gdiplus::Color(GetRValue(kButtonHoverColor), GetGValue(kButtonHoverColor), GetBValue(kButtonHoverColor))
                : Gdiplus::Color(GetRValue(kHighlightColor), GetGValue(kHighlightColor), GetBValue(kHighlightColor));
            Gdiplus::Rect langRect(langX + 4, kWindowMargin + 6, kLanguageButtonWidth - 8, kItemHeight - 12);
            DrawRoundedRectangle(graphics, langRect, 6, langBgColor);

            std::wstring langText = chineseMode ? L"中" : L"英";
            Gdiplus::SolidBrush langBrush(Gdiplus::Color(255, 255, 255));
            Gdiplus::Font langFont(L"Microsoft YaHei UI", 10.0f);
            Gdiplus::RectF langBounds;
            graphics.MeasureString(langText.c_str(), static_cast<int>(langText.length()), &langFont, Gdiplus::PointF(0, 0), &langBounds);
            int langTextX = langX + kLanguageButtonWidth / 2 - static_cast<int>(langBounds.Width) / 2;
            int langTextY = kWindowMargin + (kItemHeight - static_cast<int>(langBounds.Height)) / 2;
            graphics.DrawString(langText.c_str(), static_cast<int>(langText.length()), &langFont,
                                Gdiplus::PointF(static_cast<float>(langTextX), static_cast<float>(langTextY)), &langBrush);

            int bx = width - kWindowMargin - kButtonWidth * 3;

            bool prevEnabled = (currentPage > 0);
            Gdiplus::Color prevColor;
            if (!prevEnabled) {
                prevColor = Gdiplus::Color(GetRValue(kButtonDisabledColor), GetGValue(kButtonDisabledColor), GetBValue(kButtonDisabledColor));
            } else if (hoverButton == 0) {
                prevColor = Gdiplus::Color(GetRValue(kButtonHoverColor), GetGValue(kButtonHoverColor), GetBValue(kButtonHoverColor));
            } else {
                prevColor = Gdiplus::Color(GetRValue(kButtonColor), GetGValue(kButtonColor), GetBValue(kButtonColor));
            }
            DrawArrow(graphics, bx + kButtonWidth / 2, buttonY, true, prevColor);

            bx += kButtonWidth;
            bool nextEnabled = (currentPage + 1 < totalPages);
            Gdiplus::Color nextColor;
            if (!nextEnabled) {
                nextColor = Gdiplus::Color(GetRValue(kButtonDisabledColor), GetGValue(kButtonDisabledColor), GetBValue(kButtonDisabledColor));
            } else if (hoverButton == 1) {
                nextColor = Gdiplus::Color(GetRValue(kButtonHoverColor), GetGValue(kButtonHoverColor), GetBValue(kButtonHoverColor));
            } else {
                nextColor = Gdiplus::Color(GetRValue(kButtonColor), GetGValue(kButtonColor), GetBValue(kButtonColor));
            }
            DrawArrow(graphics, bx + kButtonWidth / 2 - 3, buttonY, false, nextColor);

            bx += kButtonWidth;
            Gdiplus::Color gearColor = (hoverButton == 2)
                ? Gdiplus::Color(GetRValue(kButtonHoverColor), GetGValue(kButtonHoverColor), GetBValue(kButtonHoverColor))
                : Gdiplus::Color(GetRValue(kButtonColor), GetGValue(kButtonColor), GetBValue(kButtonColor));
            DrawGearIcon(graphics, bx + kButtonWidth / 2, buttonY, gearColor);
        }

        // Update layered window
        POINT srcPos = {0, 0};
        SIZE size = {width, height};
        POINT dstPos = {posX, posY};
        BLENDFUNCTION blend = {};
        blend.BlendOp = AC_SRC_OVER;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        HDC hdcWnd = GetDC(hwnd);
        UpdateLayeredWindow(hwnd, hdcWnd, &dstPos, &size, memDC, &srcPos, 0, &blend, ULW_ALPHA);
        ReleaseDC(hwnd, hdcWnd);
    }

    void HandleMouseMove(int x, int y) {
        int newHover = -1;
        if (!candidates.empty()) {
            int buttonYStart = kWindowMargin;
            int buttonYEnd = kWindowMargin + kItemHeight;
            int langX = width - kWindowMargin - kButtonWidth * 3 - kLanguageButtonWidth;
            if (y >= buttonYStart && y <= buttonYEnd) {
                if (x >= langX && x <= langX + kLanguageButtonWidth) {
                    newHover = 3; // language toggle
                } else {
                    int bx = width - kWindowMargin - kButtonWidth * 3;
                    for (int i = 0; i < 3; ++i) {
                        if (x >= bx && x <= bx + kButtonWidth) {
                            if (i == 0 && currentPage == 0) {
                                // Disabled prev button.
                            } else if (i == 1 && currentPage + 1 >= totalPages) {
                                // Disabled next button.
                            } else {
                                newHover = i;
                            }
                            break;
                        }
                        bx += kButtonWidth;
                    }
                }
            }
        }
        if (newHover != hoverButton) {
            hoverButton = newHover;
            Render();
        }
    }

    void HandleMouseClick(int x, int y) {
        if (hoverButton >= 0) {
            if (clickCallback) {
                if (hoverButton == 0) {
                    clickCallback(kCandidateActionPrevPage, 0);
                } else if (hoverButton == 1) {
                    clickCallback(kCandidateActionNextPage, 0);
                } else if (hoverButton == 2) {
                    clickCallback(kCandidateActionSettings, 0);
                } else if (hoverButton == 3) {
                    clickCallback(kCandidateActionLanguageToggle, 0);
                }
            }
            return;
        }

        int itemX = kWindowMargin + kHighlightBarWidth + kItemPaddingX;
        int itemYStart = kWindowMargin;
        int itemYEnd = kWindowMargin + kItemHeight;
        for (size_t i = 0; i < candidates.size(); ++i) {
            if (x >= itemX && x <= itemX + itemWidths[i] && y >= itemYStart && y <= itemYEnd) {
                selectedIndex = static_cast<int>(i);
                Render();
                if (clickCallback) {
                    clickCallback(kCandidateActionSelect, candidates[i].index);
                }
                return;
            }
            itemX += itemWidths[i];
        }
    }
};

CandidateWindow::CandidateWindow()
    : impl_(new Impl()) {
}

CandidateWindow::~CandidateWindow() {
    delete impl_;
}

bool CandidateWindow::Create() {
    return impl_->Create();
}

void CandidateWindow::Destroy() {
    impl_->Destroy();
}

void CandidateWindow::Show() {
    impl_->Show();
}

void CandidateWindow::Hide() {
    impl_->Hide();
}

void CandidateWindow::UpdateCandidates(const std::vector<CandidateItem>& candidates) {
    impl_->UpdateCandidates(candidates);
}

void CandidateWindow::MoveTo(int x, int y) {
    impl_->MoveTo(x, y);
}

void CandidateWindow::SetPageInfo(int currentPage, int totalPages) {
    impl_->SetPageInfo(currentPage, totalPages);
}

void CandidateWindow::SetClickCallback(ClickCallback callback) {
    impl_->SetClickCallback(std::move(callback));
}

void CandidateWindow::SetLanguageIndicator(bool chineseMode) {
    impl_->SetLanguageIndicator(chineseMode);
}

} // namespace tip

#else // Non-Windows stub

namespace tip {

class CandidateWindow::Impl {
public:
    bool created = false;
    bool visible = false;
    int posX = 0;
    int posY = 0;
    std::vector<CandidateItem> candidates;
};

CandidateWindow::CandidateWindow()
    : impl_(new Impl()) {
}

CandidateWindow::~CandidateWindow() {
    delete impl_;
}

bool CandidateWindow::Create() {
    impl_->created = true;
    return true;
}

void CandidateWindow::Destroy() {
    impl_->created = false;
    impl_->visible = false;
}

void CandidateWindow::Show() {
    impl_->visible = true;
}

void CandidateWindow::Hide() {
    impl_->visible = false;
}

void CandidateWindow::UpdateCandidates(const std::vector<CandidateItem>& candidates) {
    impl_->candidates = candidates;
}

void CandidateWindow::MoveTo(int x, int y) {
    impl_->posX = x;
    impl_->posY = y;
}

void CandidateWindow::SetPageInfo(int currentPage, int totalPages) {
    (void)currentPage;
    (void)totalPages;
}

void CandidateWindow::SetClickCallback(ClickCallback callback) {
    (void)callback;
}

void CandidateWindow::SetLanguageIndicator(bool chineseMode) {
    (void)chineseMode;
}

} // namespace tip

#endif // _WIN32
