#pragma once

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

    bool Initialize();
    void Shutdown();

    void AppendPinyinChar(wchar_t ch);
    void Backspace();
    void Clear();

    std::wstring GetRawPinyin() const;
    std::vector<Candidate> GetCandidates() const;

    bool SelectCandidate(size_t index);
    std::wstring GetCommittedText();

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
