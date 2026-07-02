#pragma once

#include <string>
#include <vector>

namespace tip {

std::wstring Utf8ToWide(const std::string& utf8);
std::string WideToUtf8(const std::wstring& wide);

std::wstring GbkToWide(const std::string& gbk);
std::string WideToGbk(const std::wstring& wide);

std::vector<std::wstring> SplitWideString(const std::wstring& input, wchar_t delimiter);

std::wstring ToLowerWide(const std::wstring& input);

bool StartsWithWide(const std::wstring& str, const std::wstring& prefix);

} // namespace tip
