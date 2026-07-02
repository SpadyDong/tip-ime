#pragma once

#include <string>
#include <vector>

namespace tip {

struct CandidateItem {
    std::wstring text;
    std::wstring pinyin;
    int index;
};

class CandidateWindow {
public:
    CandidateWindow();
    ~CandidateWindow();

    bool Create();
    void Destroy();
    void Show();
    void Hide();

    void UpdateCandidates(const std::vector<CandidateItem>& candidates);
    void MoveTo(int x, int y);

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
