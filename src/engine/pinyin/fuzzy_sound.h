#pragma once

#include <string>
#include <vector>

namespace tip {

class FuzzySound {
public:
    static std::vector<std::wstring> GetVariants(const std::wstring& pinyin);
    static void EnableDefaultFuzzyPairs();
    static void Clear();
    static void AddFuzzyPair(const std::wstring& from, const std::wstring& to);
};

} // namespace tip
