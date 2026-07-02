#pragma once

#include "trie_index.h"

#include <memory>
#include <string>
#include <vector>

namespace tip {

class Dictionary {
public:
    Dictionary();
    ~Dictionary();

    bool LoadFromFile(const std::wstring& filePath);
    bool SaveToFile(const std::wstring& filePath) const;

    void AddEntry(const std::wstring& pinyin, const std::wstring& text, int frequency);
    bool RemoveEntry(const std::wstring& pinyin, const std::wstring& text);
    void UpdateFrequency(const std::wstring& pinyin, const std::wstring& text, int delta);

    std::vector<DictEntry> Query(const std::wstring& pinyinPrefix) const;
    size_t Size() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
