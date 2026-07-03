#include "string_utils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <codecvt>
#include <locale>
#endif

namespace tip {

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return std::wstring();
    }
#ifdef _WIN32
    int size = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), -1, nullptr, 0);
    if (size <= 0) {
        return std::wstring();
    }
    std::wstring result(size - 1, L'\0');
    ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), -1, &result[0], size);
    return result;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(utf8);
#endif
}

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return std::string();
    }
#ifdef _WIN32
    int size = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return std::string();
    }
    std::string result(size - 1, '\0');
    ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wide);
#endif
}

std::wstring GbkToWide(const std::string& gbk) {
    if (gbk.empty()) {
        return std::wstring();
    }
#ifdef _WIN32
    int size = ::MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, nullptr, 0);
    if (size <= 0) {
        return std::wstring();
    }
    std::wstring result(size - 1, L'\0');
    ::MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, &result[0], size);
    return result;
#else
    return Utf8ToWide(gbk);
#endif
}

std::string WideToGbk(const std::wstring& wide) {
    if (wide.empty()) {
        return std::string();
    }
#ifdef _WIN32
    int size = ::WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return std::string();
    }
    std::string result(size - 1, '\0');
    ::WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
#else
    return WideToUtf8(wide);
#endif
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
