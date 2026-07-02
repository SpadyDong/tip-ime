#pragma once

#include <functional>
#include <string>
#include <vector>

namespace tip {

struct CandidateItem {
    std::wstring text;
    std::wstring pinyin;
    int index;
};

// Action codes delivered through the click callback.
constexpr int kCandidateActionPrevPage = 0;
constexpr int kCandidateActionNextPage = 1;
constexpr int kCandidateActionSettings = 2;
constexpr int kCandidateActionSelect = 3;

class CandidateWindow {
public:
    using ClickCallback = std::function<void(int action, int param)>;

    CandidateWindow();
    ~CandidateWindow();

    bool Create();
    void Destroy();
    void Show();
    void Hide();

    void UpdateCandidates(const std::vector<CandidateItem>& candidates);
    void MoveTo(int x, int y);

    void SetPageInfo(int currentPage, int totalPages);
    void SetClickCallback(ClickCallback callback);

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
