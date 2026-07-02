#include "dictionary.h"

#include <fstream>
#include <sstream>

#include "logger.h"
#include "string_utils.h"

namespace tip {

class Dictionary::Impl {
public:
    TrieIndex trie;
    size_t size = 0;
};

Dictionary::Dictionary()
    : impl_(new Impl()) {
}

Dictionary::~Dictionary() {
    delete impl_;
}

bool Dictionary::LoadFromFile(const std::wstring& filePath) {
    std::string utf8Path = WideToUtf8(filePath);
    std::ifstream file(utf8Path);
    if (!file.is_open()) {
        TIP_LOG_ERROR(L"Failed to open dictionary file: " + filePath);
        return false;
    }

    std::string line;
    size_t loaded = 0;
    while (std::getline(file, line)) {
        if (line.empty() || line.front() == '#') {
            continue;
        }

        std::wstring wideLine = Utf8ToWide(line);
        auto parts = SplitWideString(wideLine, L'\t');
        if (parts.size() < 2) {
            continue;
        }

        std::wstring pinyin = ToLowerWide(parts[0]);
        std::wstring text = parts[1];
        int frequency = 0;
        if (parts.size() >= 3) {
            try {
                frequency = std::stoi(parts[2]);
            } catch (...) {
                frequency = 0;
            }
        }

        AddEntry(pinyin, text, frequency);
        ++loaded;
    }

    TIP_LOG_INFO(L"Dictionary loaded " + std::to_wstring(loaded) + L" entries from: " + filePath);
    return true;
}

bool Dictionary::SaveToFile(const std::wstring& filePath) const {
    // TODO: implement binary dictionary serialization
    TIP_LOG_INFO(L"Dictionary saving to: " + filePath);
    return true;
}

void Dictionary::AddEntry(const std::wstring& pinyin, const std::wstring& text, int frequency) {
    impl_->trie.Insert(pinyin, { text, pinyin, frequency });
    ++impl_->size;
}

bool Dictionary::RemoveEntry(const std::wstring& pinyin, const std::wstring& text) {
    if (impl_->trie.Remove(pinyin, text)) {
        --impl_->size;
        return true;
    }
    return false;
}

void Dictionary::UpdateFrequency(const std::wstring& pinyin, const std::wstring& text, int delta) {
    // TODO: update frequency in-place
    (void)pinyin;
    (void)text;
    (void)delta;
}

std::vector<DictEntry> Dictionary::Query(const std::wstring& pinyinPrefix) const {
    return impl_->trie.Search(pinyinPrefix);
}

size_t Dictionary::Size() const {
    return impl_->size;
}

} // namespace tip
