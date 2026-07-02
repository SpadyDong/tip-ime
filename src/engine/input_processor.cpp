#include "input_processor.h"

#include <algorithm>
#include <unordered_set>

#include "config_manager.h"
#include "dictionary.h"
#include "fuzzy_sound.h"
#include "logger.h"
#include "pinyin_parser.h"

namespace tip {

class InputProcessor::Impl {
public:
    std::wstring rawPinyin;
    std::wstring committedText;
    Dictionary dictionary;
    bool initialized = false;
    size_t pageSize = 5;
    size_t currentPage = 0;
    bool enableJianpin = true;

    void ResetPage() {
        currentPage = 0;
    }

    std::vector<std::wstring> BuildQueryKeys(const std::wstring& raw) const {
        std::vector<std::wstring> keys;
        if (raw.empty()) {
            return keys;
        }

        // Full pinyin as typed (e.g., "zhongguo").
        keys.push_back(raw);

        // Segmented full pinyin joined by separators is not needed here because
        // the dictionary stores pinyin without separators.
        auto segments = PinyinParser::Segment(raw);

        // Jianpin combinations from the segmented syllables.
        if (enableJianpin) {
            std::wstring jianpin;
            for (const auto& seg : segments) {
                if (!seg.empty()) {
                    jianpin.push_back(seg.front());
                }
            }
            if (!jianpin.empty() && jianpin != raw) {
                keys.push_back(jianpin);
            }
        }

        // Fuzzy variants for the first syllable.
        if (!segments.empty()) {
            auto variants = FuzzySound::GetVariants(segments[0]);
            for (const auto& variant : variants) {
                std::wstring variantKey = variant;
                for (size_t i = 1; i < segments.size(); ++i) {
                    variantKey += segments[i];
                }
                keys.push_back(variantKey);
            }
        }

        return keys;
    }
};

InputProcessor::InputProcessor()
    : impl_(new Impl()) {
}

InputProcessor::~InputProcessor() {
    delete impl_;
}

bool InputProcessor::Initialize() {
    auto& config = ConfigManager::Instance().GetConfig();

    if (config.enableFuzzySound) {
        FuzzySound::EnableDefaultFuzzyPairs();
    } else {
        FuzzySound::Clear();
    }

    impl_->enableJianpin = config.enableJianpin;
    impl_->pageSize = static_cast<size_t>(config.candidateCount);
    if (impl_->pageSize < 1 || impl_->pageSize > 9) {
        impl_->pageSize = 5;
    }

    // Load the bundled base dictionary. The file is located relative to the
    // project root and uses tab-separated UTF-8 text.
    std::wstring dictPath = L"data/dictionary/base_dict.txt";
    if (!impl_->dictionary.LoadFromFile(dictPath)) {
        TIP_LOG_WARNING(L"Failed to load base dictionary, continuing with empty dictionary");
    }

    impl_->initialized = true;
    impl_->ResetPage();
    TIP_LOG_INFO(L"InputProcessor initialized with dictionary size: " +
                 std::to_wstring(impl_->dictionary.Size()));
    return true;
}

void InputProcessor::Shutdown() {
    impl_->initialized = false;
    TIP_LOG_INFO(L"InputProcessor shutdown");
}

void InputProcessor::AppendPinyinChar(wchar_t ch) {
    if (ch >= L'a' && ch <= L'z') {
        impl_->rawPinyin.push_back(ch);
        impl_->ResetPage();
    }
}

void InputProcessor::Backspace() {
    if (!impl_->rawPinyin.empty()) {
        impl_->rawPinyin.pop_back();
        impl_->ResetPage();
    }
}

void InputProcessor::Clear() {
    impl_->rawPinyin.clear();
    impl_->committedText.clear();
    impl_->ResetPage();
}

std::wstring InputProcessor::GetRawPinyin() const {
    return impl_->rawPinyin;
}

std::vector<Candidate> InputProcessor::GetCandidates() const {
    std::vector<Candidate> result;
    if (impl_->rawPinyin.empty()) {
        return result;
    }

    auto keys = impl_->BuildQueryKeys(impl_->rawPinyin);
    std::unordered_set<std::wstring> seen;

    for (const auto& key : keys) {
        auto entries = impl_->dictionary.Query(key);
        for (const auto& entry : entries) {
            if (seen.find(entry.text) == seen.end()) {
                seen.insert(entry.text);
                result.push_back({ entry.text, entry.pinyin, entry.frequency });
            }
        }
    }

    // If no dictionary entries match, fall back to showing the raw pinyin so
    // the user still sees feedback.
    if (result.empty()) {
        result.push_back({ impl_->rawPinyin, impl_->rawPinyin, 0 });
    }

    std::sort(result.begin(), result.end(), [](const Candidate& a, const Candidate& b) {
        return a.frequency > b.frequency;
    });

    return result;
}

void InputProcessor::SetPageSize(size_t pageSize) {
    impl_->pageSize = pageSize;
    if (impl_->pageSize < 1) {
        impl_->pageSize = 1;
    } else if (impl_->pageSize > 9) {
        impl_->pageSize = 9;
    }
    impl_->ResetPage();
}

size_t InputProcessor::GetPageSize() const {
    return impl_->pageSize;
}

bool InputProcessor::PageDown() {
    size_t totalPages = GetTotalPages();
    if (impl_->currentPage + 1 >= totalPages) {
        return false;
    }
    ++impl_->currentPage;
    return true;
}

bool InputProcessor::PageUp() {
    if (impl_->currentPage == 0) {
        return false;
    }
    --impl_->currentPage;
    return true;
}

size_t InputProcessor::GetCurrentPage() const {
    return impl_->currentPage;
}

size_t InputProcessor::GetTotalPages() const {
    auto candidates = GetCandidates();
    if (candidates.empty()) {
        return 0;
    }
    return (candidates.size() + impl_->pageSize - 1) / impl_->pageSize;
}

std::vector<Candidate> InputProcessor::GetPagedCandidates() const {
    auto candidates = GetCandidates();
    if (candidates.empty()) {
        return candidates;
    }

    size_t start = impl_->currentPage * impl_->pageSize;
    if (start >= candidates.size()) {
        start = 0;
    }
    size_t end = start + impl_->pageSize;
    if (end > candidates.size()) {
        end = candidates.size();
    }

    return std::vector<Candidate>(candidates.begin() + start, candidates.begin() + end);
}

bool InputProcessor::SelectCandidate(size_t index) {
    auto candidates = GetCandidates();
    size_t globalIndex = impl_->currentPage * impl_->pageSize + index;
    if (globalIndex >= candidates.size()) {
        return false;
    }
    impl_->committedText = candidates[globalIndex].text;
    impl_->rawPinyin.clear();
    impl_->ResetPage();
    return true;
}

std::wstring InputProcessor::GetCommittedText() {
    std::wstring text = impl_->committedText;
    impl_->committedText.clear();
    return text;
}

} // namespace tip
