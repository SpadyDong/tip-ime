#include "input_processor.h"

#include "logger.h"

namespace tip {

class InputProcessor::Impl {
public:
    std::wstring rawPinyin;
    std::wstring committedText;
};

InputProcessor::InputProcessor()
    : impl_(new Impl()) {
}

InputProcessor::~InputProcessor() {
    delete impl_;
}

bool InputProcessor::Initialize() {
    TIP_LOG_INFO(L"InputProcessor initialized");
    return true;
}

void InputProcessor::Shutdown() {
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
    // TODO: query dictionary engine for real candidates
    std::vector<Candidate> result;
    if (!impl_->rawPinyin.empty()) {
        result.push_back({ impl_->rawPinyin, impl_->rawPinyin, 0 });
    }
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
