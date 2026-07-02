#include "dictionary.h"

#include "logger.h"

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
    // TODO: implement binary dictionary deserialization
    TIP_LOG_INFO(L"Dictionary loading from: " + filePath);
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
