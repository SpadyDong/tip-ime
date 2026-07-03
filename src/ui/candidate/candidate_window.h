#pragma once

#include <string>
#include <vector>

namespace tip {

struct CandidateItem {
    std::wstring text;
    std::wstring pinyin;
    int index;
};

struct CandidateWindowStyle {
    unsigned int backgroundColor = 0xFFFFFF;
    unsigned int borderColor = 0xCCCCCC;
    unsigned int textColor = 0x333333;
    unsigned int highlightColor = 0xE6F7FF;
    unsigned int selectionBackgroundColor = 0x666666;
    unsigned int selectionTextColor = 0xFFFFFF;
    std::wstring fontFamily = L"Microsoft YaHei UI";
    int fontSize = 14;
};

bool LoadCandidateWindowStyleFromFile(const std::wstring& filePath, CandidateWindowStyle& style);

class CandidateWindow {
public:
    CandidateWindow();
    ~CandidateWindow();

    bool Create();
    void Destroy();
    void Show();
    void Hide();

    void SetCornerRadius(int radius);
    void SetStyle(const CandidateWindowStyle& style);
    void UpdateCandidates(const std::vector<CandidateItem>& candidates, size_t selectedIndex);
    void MoveTo(int x, int y);

    bool IsVisible() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
