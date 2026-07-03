#pragma once

#include <memory>
#include <string>
#include <vector>

namespace tip {

struct Candidate {
    std::wstring text;
    std::wstring pinyin;
    int frequency;
};

class InputProcessor {
public:
    InputProcessor();
    ~InputProcessor();

    bool Initialize(const std::wstring& dictionaryPath);
    void Shutdown();

    void AppendPinyinChar(wchar_t ch);
    void Backspace();
    void Clear();

    std::wstring GetRawPinyin() const;
    std::vector<Candidate> GetCandidates() const;

    bool PageDown();
    bool PageUp();
    std::vector<Candidate> GetPageCandidates() const;
    int GetCurrentPage() const;
    int GetPageCount() const;

    void SetSelectedIndex(size_t index);
    size_t GetSelectedIndex() const;

    bool SelectCurrentCandidate();
    bool SelectCandidateByIndex(size_t index);
    std::wstring GetCommittedText();

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
