#include "fuzzy_sound.h"

#include <unordered_map>

namespace tip {

namespace {

std::unordered_map<std::wstring, std::vector<std::wstring>>& FuzzyMap() {
    static std::unordered_map<std::wstring, std::vector<std::wstring>> s_map;
    return s_map;
}

} // namespace

std::vector<std::wstring> FuzzySound::GetVariants(const std::wstring& pinyin) {
    auto it = FuzzyMap().find(pinyin);
    if (it != FuzzyMap().end()) {
        return it->second;
    }
    return {};
}

void FuzzySound::EnableDefaultFuzzyPairs() {
    Clear();
    AddFuzzyPair(L"z", L"zh");
    AddFuzzyPair(L"zh", L"z");
    AddFuzzyPair(L"c", L"ch");
    AddFuzzyPair(L"ch", L"c");
    AddFuzzyPair(L"s", L"sh");
    AddFuzzyPair(L"sh", L"s");
    AddFuzzyPair(L"n", L"l");
    AddFuzzyPair(L"l", L"n");
    AddFuzzyPair(L"f", L"h");
    AddFuzzyPair(L"h", L"f");
    AddFuzzyPair(L"r", L"l");
    AddFuzzyPair(L"l", L"r");
}

void FuzzySound::Clear() {
    FuzzyMap().clear();
}

void FuzzySound::AddFuzzyPair(const std::wstring& from, const std::wstring& to) {
    FuzzyMap()[from].push_back(to);
}

} // namespace tip
