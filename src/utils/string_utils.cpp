#include "string_utils.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>

namespace tip {

namespace {

bool DecodeUtf8CodePoint(const std::string& utf8, size_t& pos, char32_t& codePoint) {
    if (pos >= utf8.size()) {
        return false;
    }
    unsigned char c0 = static_cast<unsigned char>(utf8[pos]);
    if ((c0 & 0x80) == 0) {
        codePoint = c0;
        pos += 1;
        return true;
    }
    if ((c0 & 0xE0) == 0xC0) {
        if (pos + 1 > utf8.size()) {
            return false;
        }
        codePoint = (static_cast<char32_t>(c0 & 0x1F) << 6) |
                    (static_cast<unsigned char>(utf8[pos + 1]) & 0x3F);
        pos += 2;
        return true;
    }
    if ((c0 & 0xF0) == 0xE0) {
        if (pos + 2 > utf8.size()) {
            return false;
        }
        codePoint = (static_cast<char32_t>(c0 & 0x0F) << 12) |
                    ((static_cast<unsigned char>(utf8[pos + 1]) & 0x3F) << 6) |
                    (static_cast<unsigned char>(utf8[pos + 2]) & 0x3F);
        pos += 3;
        return true;
    }
    if ((c0 & 0xF8) == 0xF0) {
        if (pos + 3 > utf8.size()) {
            return false;
        }
        codePoint = (static_cast<char32_t>(c0 & 0x07) << 18) |
                    ((static_cast<unsigned char>(utf8[pos + 1]) & 0x3F) << 12) |
                    ((static_cast<unsigned char>(utf8[pos + 2]) & 0x3F) << 6) |
                    (static_cast<unsigned char>(utf8[pos + 3]) & 0x3F);
        pos += 4;
        return true;
    }
    return false;
}

void AppendCodePoint(std::wstring& out, char32_t codePoint) {
    if constexpr (sizeof(wchar_t) == 2) {
        // UTF-16: use surrogate pair for code points above U+FFFF.
        if (codePoint > 0x10FFFF) {
            return;
        }
        if (codePoint <= 0xFFFF) {
            out.push_back(static_cast<wchar_t>(codePoint));
        } else {
            codePoint -= 0x10000;
            wchar_t high = static_cast<wchar_t>(0xD800 + (codePoint >> 10));
            wchar_t low = static_cast<wchar_t>(0xDC00 + (codePoint & 0x3FF));
            out.push_back(high);
            out.push_back(low);
        }
    } else {
        // UTF-32 / other wide encodings: store directly.
        out.push_back(static_cast<wchar_t>(codePoint));
    }
}

} // namespace

std::wstring Utf8ToWide(const std::string& utf8) {
    std::wstring result;
    result.reserve(utf8.size());
    size_t pos = 0;
    char32_t codePoint = 0;
    while (DecodeUtf8CodePoint(utf8, pos, codePoint)) {
        AppendCodePoint(result, codePoint);
    }
    return result;
}

std::string WideToUtf8(const std::wstring& wide) {
    std::string result;
    result.reserve(wide.size() * 3);
    for (size_t i = 0; i < wide.size(); ++i) {
        char32_t codePoint = 0;
        if constexpr (sizeof(wchar_t) == 2) {
            wchar_t c = wide[i];
            if (c >= 0xD800 && c <= 0xDBFF && i + 1 < wide.size()) {
                wchar_t low = wide[i + 1];
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    codePoint = 0x10000 + ((static_cast<char32_t>(c) - 0xD800) << 10) +
                                (static_cast<char32_t>(low) - 0xDC00);
                    ++i;
                } else {
                    codePoint = c;
                }
            } else {
                codePoint = c;
            }
        } else {
            codePoint = static_cast<char32_t>(wide[i]);
        }

        if (codePoint <= 0x7F) {
            result.push_back(static_cast<char>(codePoint));
        } else if (codePoint <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else if (codePoint <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else if (codePoint <= 0x10FFFF) {
            result.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        }
    }
    return result;
}

std::wstring GbkToWide(const std::string& gbk) {
    // GBK conversion is Windows-specific; fall back to UTF-8 interpretation for now.
    return Utf8ToWide(gbk);
}

std::string WideToGbk(const std::wstring& wide) {
    // GBK conversion is Windows-specific; fall back to UTF-8 interpretation for now.
    return WideToUtf8(wide);
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
