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
    bool ImportFromFile(const std::wstring& filePath);

    void AddEntry(const std::wstring& pinyin, const std::wstring& text, int frequency);
    bool RemoveEntry(const std::wstring& pinyin, const std::wstring& text);
    void UpdateFrequency(const std::wstring& pinyin, const std::wstring& text, int delta);
    bool HasEntry(const std::wstring& pinyin, const std::wstring& text) const;

    std::vector<DictEntry> Query(const std::wstring& pinyinPrefix) const;
    std::vector<DictEntry> GetAllEntries() const;
    size_t Size() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
