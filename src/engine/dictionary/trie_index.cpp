#include "trie_index.h"

#include <algorithm>
#include <map>
#include <memory>

namespace tip {

struct TrieNode {
    std::map<wchar_t, std::unique_ptr<TrieNode>> children;
    std::vector<DictEntry> entries;
};

class TrieIndex::Impl {
public:
    std::unique_ptr<TrieNode> root = std::make_unique<TrieNode>();

    void CollectEntries(const TrieNode* node, std::vector<DictEntry>& result) const {
        if (!node) {
            return;
        }
        result.insert(result.end(), node->entries.begin(), node->entries.end());
        for (const auto& [_, child] : node->children) {
            CollectEntries(child.get(), result);
        }
    }
};

TrieIndex::TrieIndex()
    : impl_(new Impl()) {
}

TrieIndex::~TrieIndex() {
    delete impl_;
}

void TrieIndex::Insert(const std::wstring& pinyin, const DictEntry& entry) {
    TrieNode* node = impl_->root.get();
    for (wchar_t ch : pinyin) {
        auto it = node->children.find(ch);
        if (it == node->children.end()) {
            it = node->children.emplace(ch, std::make_unique<TrieNode>()).first;
        }
        node = it->second.get();
    }
    node->entries.push_back(entry);
}

std::vector<DictEntry> TrieIndex::Search(const std::wstring& pinyinPrefix) const {
    std::vector<DictEntry> result;
    const TrieNode* node = impl_->root.get();
    for (wchar_t ch : pinyinPrefix) {
        auto it = node->children.find(ch);
        if (it == node->children.end()) {
            return result;
        }
        node = it->second.get();
    }
    impl_->CollectEntries(node, result);
    std::sort(result.begin(), result.end(), [](const DictEntry& a, const DictEntry& b) {
        return a.frequency > b.frequency;
    });
    return result;
}

bool TrieIndex::Remove(const std::wstring& pinyin, const std::wstring& text) {
    TrieNode* node = impl_->root.get();
    for (wchar_t ch : pinyin) {
        auto it = node->children.find(ch);
        if (it == node->children.end()) {
            return false;
        }
        node = it->second.get();
    }
    auto it = std::remove_if(node->entries.begin(), node->entries.end(),
        [&text](const DictEntry& entry) { return entry.text == text; });
    if (it == node->entries.end()) {
        return false;
    }
    node->entries.erase(it, node->entries.end());
    return true;
}

} // namespace tip
