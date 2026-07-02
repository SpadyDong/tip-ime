#pragma once

#include <string>
#include <vector>

namespace tip {

class PinyinParser {
public:
    static std::vector<std::wstring> Segment(const std::wstring& rawPinyin);
    static bool IsValidPinyin(const std::wstring& pinyin);
    static std::vector<std::wstring> GetJianpinCombinations(const std::wstring& rawPinyin);
};

} // namespace tip
