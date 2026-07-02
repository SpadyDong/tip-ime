#pragma once

#include <cstddef>
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

    void SetPageSize(size_t pageSize);
    size_t GetPageSize() const;

    bool PageDown();
    bool PageUp();
    size_t GetCurrentPage() const;
    size_t GetTotalPages() const;
    std::vector<Candidate> GetPagedCandidates() const;

    bool SelectCandidate(size_t index);
    std::wstring GetCommittedText();

    bool ImportUserPhrases(const std::wstring& filePath);
    bool ExportUserPhrases(const std::wstring& filePath) const;
    bool AddUserPhrase(const std::wstring& pinyin, const std::wstring& text, int frequency);

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
