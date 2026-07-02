#include "dictionary.h"

#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <filesystem>
#endif

#include "logger.h"
#include "string_utils.h"

namespace tip {

namespace {

#ifdef _WIN32
bool CreateDirectoryRecursive(const std::wstring& path) {
    if (path.empty()) {
        return true;
    }

    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    size_t sep = path.find_last_of(L"\\/");
    if (sep != std::wstring::npos) {
        if (!CreateDirectoryRecursive(path.substr(0, sep))) {
            return false;
        }
    }

    return CreateDirectoryW(path.c_str(), nullptr) != 0 ||
           GetLastError() == ERROR_ALREADY_EXISTS;
}
#endif

void EnsureParentDirectoryExists(const std::wstring& filePath) {
    size_t lastSep = filePath.find_last_of(L"\\/");
    if (lastSep == std::wstring::npos) {
        return;
    }

    std::wstring parent = filePath.substr(0, lastSep);
#ifdef _WIN32
    CreateDirectoryRecursive(parent);
#else
    std::filesystem::create_directories(std::filesystem::path(parent));
#endif
}

} // namespace

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
    std::string utf8Path = WideToUtf8(filePath);

    // Ensure the parent directory exists so that user dictionaries can be
    // created on first use even when the data directory has not been set up.
    EnsureParentDirectoryExists(filePath);

    std::ofstream file(utf8Path);
    if (!file.is_open()) {
        TIP_LOG_ERROR(L"Failed to open dictionary file for writing: " + filePath);
        return false;
    }

    file << "# TIP User Dictionary\n";
    auto entries = GetAllEntries();
    for (const auto& entry : entries) {
        file << WideToUtf8(entry.pinyin) << "\t"
             << WideToUtf8(entry.text) << "\t"
             << entry.frequency << "\n";
    }

    TIP_LOG_INFO(L"Dictionary saved " + std::to_wstring(entries.size()) +
                 L" entries to: " + filePath);
    return true;
}

bool Dictionary::ImportFromFile(const std::wstring& filePath) {
    std::string utf8Path = WideToUtf8(filePath);
    std::ifstream file(utf8Path);
    if (!file.is_open()) {
        TIP_LOG_ERROR(L"Failed to open import dictionary file: " + filePath);
        return false;
    }

    std::string line;
    size_t imported = 0;
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

        if (!HasEntry(pinyin, text)) {
            AddEntry(pinyin, text, frequency);
            ++imported;
        }
    }

    TIP_LOG_INFO(L"Dictionary imported " + std::to_wstring(imported) +
                 L" entries from: " + filePath);
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

bool Dictionary::HasEntry(const std::wstring& pinyin, const std::wstring& text) const {
    auto entries = impl_->trie.Search(pinyin);
    for (const auto& entry : entries) {
        if (entry.pinyin == pinyin && entry.text == text) {
            return true;
        }
    }
    return false;
}

std::vector<DictEntry> Dictionary::Query(const std::wstring& pinyinPrefix) const {
    return impl_->trie.Search(pinyinPrefix);
}

std::vector<DictEntry> Dictionary::GetAllEntries() const {
    return impl_->trie.GetAllEntries();
}

size_t Dictionary::Size() const {
    return impl_->size;
}

} // namespace tip
