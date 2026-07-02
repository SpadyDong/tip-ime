#include "utils/string_utils.h"

#include <vector>

using namespace tip;

struct TestResult {
    const char* name;
    bool passed;
};

#define CHECK(name, condition) results.push_back({ name, (condition) })

std::vector<TestResult> RunStringUtilsTests() {
    std::vector<TestResult> results;

    std::string utf8 = "\xE4\xB8\xAD\xE5\x9B\xBD"; // "中国" in UTF-8
    std::wstring wide = Utf8ToWide(utf8);
    CHECK("Utf8ToWide converts '中国'", wide == L"中国");

    std::string back = WideToUtf8(L"中国");
    CHECK("WideToUtf8 converts '中国'", back == utf8);

    auto parts = SplitWideString(L"a\tb\tc", L'\t');
    CHECK("SplitWideString splits 3 parts", parts.size() == 3 && parts[0] == L"a" && parts[1] == L"b" && parts[2] == L"c");

    CHECK("ToLowerWide lowercases", ToLowerWide(L"ABC") == L"abc");
    CHECK("StartsWithWide matches prefix", StartsWithWide(L"hello", L"hel"));
    CHECK("StartsWithWide rejects longer prefix", !StartsWithWide(L"hi", L"hello"));

    return results;
}
