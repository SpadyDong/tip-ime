#include "input_processor.h"

#include <algorithm>
#include <unordered_set>

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
        std::wstring jianpin;
        for (const auto& seg : segments) {
            if (!seg.empty()) {
                jianpin.push_back(seg.front());
            }
        }
        if (!jianpin.empty() && jianpin != raw) {
            keys.push_back(jianpin);
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
    FuzzySound::EnableDefaultFuzzyPairs();

    // Load the bundled base dictionary. The file is located relative to the
    // project root and uses tab-separated UTF-8 text.
    std::wstring dictPath = L"data/dictionary/base_dict.txt";
    if (!impl_->dictionary.LoadFromFile(dictPath)) {
        TIP_LOG_WARNING(L"Failed to load base dictionary, continuing with empty dictionary");
    }

    impl_->initialized = true;
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
    }
}

void InputProcessor::Backspace() {
    if (!impl_->rawPinyin.empty()) {
        impl_->rawPinyin.pop_back();
    }
}

void InputProcessor::Clear() {
    impl_->rawPinyin.clear();
    impl_->committedText.clear();
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

bool InputProcessor::SelectCandidate(size_t index) {
    auto candidates = GetCandidates();
    if (index >= candidates.size()) {
        return false;
    }
    impl_->committedText = candidates[index].text;
    impl_->rawPinyin.clear();
    return true;
}

std::wstring InputProcessor::GetCommittedText() {
    std::wstring text = impl_->committedText;
    impl_->committedText.clear();
    return text;
}

} // namespace tip
