#include "string_utils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace tip {

std::wstring Utf8ToWide(const std::string& utf8) {
    // TODO: implement platform-specific UTF-8 to UTF-16 conversion
    return std::wstring(utf8.begin(), utf8.end());
}

std::string WideToUtf8(const std::wstring& wide) {
    // TODO: implement platform-specific UTF-16 to UTF-8 conversion
    return std::string(wide.begin(), wide.end());
}

std::wstring GbkToWide(const std::string& gbk) {
    // TODO: implement platform-specific GBK to UTF-16 conversion
    return std::wstring(gbk.begin(), gbk.end());
}

std::string WideToGbk(const std::wstring& wide) {
    // TODO: implement platform-specific UTF-16 to GBK conversion
    return std::string(wide.begin(), wide.end());
}

std::vector<std::wstring> SplitWideString(const std::wstring& input, wchar_t delimiter) {
    std::vector<std::wstring> result;
    std::wstringstream ss(input);
    std::wstring token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            result.push_back(token);
        }
    }
    return result;
}

std::wstring ToLowerWide(const std::wstring& input) {
    std::wstring result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::towlower);
    return result;
}

bool StartsWithWide(const std::wstring& str, const std::wstring& prefix) {
    if (prefix.size() > str.size()) {
        return false;
    }
    return str.compare(0, prefix.size(), prefix) == 0;
}

} // namespace tip
