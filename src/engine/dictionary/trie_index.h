#pragma once

#include <memory>
#include <string>
#include <vector>

namespace tip {

struct DictEntry {
    std::wstring text;
    std::wstring pinyin;
    int frequency;
};

class TrieIndex {
public:
    TrieIndex();
    ~TrieIndex();

    void Insert(const std::wstring& pinyin, const DictEntry& entry);
    std::vector<DictEntry> Search(const std::wstring& pinyinPrefix) const;
    bool Remove(const std::wstring& pinyin, const std::wstring& text);
    std::vector<DictEntry> GetAllEntries() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
