#include "input_processor.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>

#include "config/config_manager.h"
#include "dictionary/dictionary.h"
#include "logger.h"
#include "pinyin/pinyin_parser.h"
#include "utils/string_utils.h"

namespace tip {

class InputProcessor::Impl {
public:
    std::wstring rawPinyin;
    std::wstring committedText;
    std::unique_ptr<Dictionary> dictionary;
    int currentPage = 0;
    size_t selectedIndex = 0;

    int PageSize() const {
        return std::max(1, ConfigManager::Instance().GetConfig().candidateCount);
    }

    void ResetPaging() {
        currentPage = 0;
        selectedIndex = 0;
    }

    void ClampSelection(size_t pageSize) {
        if (selectedIndex >= pageSize) {
            selectedIndex = 0;
        }
    }
};

InputProcessor::InputProcessor()
    : impl_(new Impl()) {
}

InputProcessor::~InputProcessor() {
    delete impl_;
}

bool InputProcessor::Initialize(const std::wstring& dictionaryPath) {
    impl_->dictionary = std::make_unique<Dictionary>();
    if (!impl_->dictionary->LoadFromFile(dictionaryPath)) {
        TIP_LOG_INFO(L"InputProcessor: failed to load dictionary from " + dictionaryPath);
    }

    std::ifstream file(WideToUtf8(dictionaryPath));
    if (file.is_open()) {
        std::string lineUtf8;
        while (std::getline(file, lineUtf8)) {
            std::wstring line = Utf8ToWide(lineUtf8);
            if (line.empty() || line[0] == L'#') {
                continue;
            }
            auto parts = SplitWideString(line, L'\t');
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
            impl_->dictionary->AddEntry(pinyin, text, frequency);
        }
    }

    TIP_LOG_INFO(L"InputProcessor initialized with dictionary: " + dictionaryPath);
    return true;
}

void InputProcessor::Shutdown() {
    impl_->dictionary.reset();
    TIP_LOG_INFO(L"InputProcessor shutdown");
}

void InputProcessor::AppendPinyinChar(wchar_t ch) {
    if (ch >= L'A' && ch <= L'Z') {
        ch = static_cast<wchar_t>(ch - L'A' + L'a');
    }
    if (ch >= L'a' && ch <= L'z') {
        impl_->rawPinyin.push_back(ch);
        impl_->ResetPaging();
    }
}

void InputProcessor::Backspace() {
    if (!impl_->rawPinyin.empty()) {
        impl_->rawPinyin.pop_back();
        impl_->ResetPaging();
    }
}

void InputProcessor::Clear() {
    impl_->rawPinyin.clear();
    impl_->committedText.clear();
    impl_->ResetPaging();
}

std::wstring InputProcessor::GetRawPinyin() const {
    return impl_->rawPinyin;
}

std::vector<Candidate> InputProcessor::GetCandidates() const {
    std::vector<Candidate> result;
    if (!impl_->rawPinyin.empty()) {
        if (impl_->dictionary) {
            auto entries = impl_->dictionary->Query(impl_->rawPinyin);
            for (const auto& entry : entries) {
                result.push_back({ entry.text, entry.pinyin, entry.frequency });
            }
        }
        if (result.empty()) {
            result.push_back({ impl_->rawPinyin, impl_->rawPinyin, 0 });
        }
    }
    return result;
}

bool InputProcessor::PageDown() {
    int pageCount = GetPageCount();
    if (impl_->currentPage + 1 < pageCount) {
        ++impl_->currentPage;
        impl_->selectedIndex = 0;
        return true;
    }
    return false;
}

bool InputProcessor::PageUp() {
    if (impl_->currentPage > 0) {
        --impl_->currentPage;
        impl_->selectedIndex = 0;
        return true;
    }
    return false;
}

std::vector<Candidate> InputProcessor::GetPageCandidates() const {
    auto all = GetCandidates();
    int pageSize = impl_->PageSize();
    int start = impl_->currentPage * pageSize;
    if (start >= static_cast<int>(all.size())) {
        return {};
    }
    int end = std::min(start + pageSize, static_cast<int>(all.size()));
    return std::vector<Candidate>(all.begin() + start, all.begin() + end);
}

int InputProcessor::GetCurrentPage() const {
    return impl_->currentPage;
}

int InputProcessor::GetPageCount() const {
    int pageSize = impl_->PageSize();
    auto all = GetCandidates();
    return static_cast<int>(std::ceil(static_cast<double>(all.size()) / pageSize));
}

void InputProcessor::SetSelectedIndex(size_t index) {
    impl_->selectedIndex = index;
}

size_t InputProcessor::GetSelectedIndex() const {
    return impl_->selectedIndex;
}

bool InputProcessor::SelectCurrentCandidate() {
    return SelectCandidateByIndex(impl_->selectedIndex);
}

bool InputProcessor::SelectCandidateByIndex(size_t index) {
    auto page = GetPageCandidates();
    if (index >= page.size()) {
        return false;
    }
    impl_->committedText = page[index].text;
    impl_->rawPinyin.clear();
    impl_->ResetPaging();
    return true;
}

std::wstring InputProcessor::GetCommittedText() {
    std::wstring text = impl_->committedText;
    impl_->committedText.clear();
    return text;
}

} // namespace tip
