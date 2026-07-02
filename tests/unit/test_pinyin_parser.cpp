#include "engine/pinyin/pinyin_parser.h"

#include <vector>

using namespace tip;

struct TestResult {
    const char* name;
    bool passed;
};

#define CHECK(name, condition) results.push_back({ name, (condition) })

std::vector<TestResult> RunPinyinParserTests() {
    std::vector<TestResult> results;

    auto seg1 = PinyinParser::Segment(L"zhongguo");
    CHECK("segment zhongguo into 2 syllables", seg1.size() == 2 && seg1[0] == L"zhong" && seg1[1] == L"guo");

    auto seg2 = PinyinParser::Segment(L"beijing");
    CHECK("segment beijing into 2 syllables", seg2.size() == 2 && seg2[0] == L"bei" && seg2[1] == L"jing");

    CHECK("valid pinyin 'zhuang'", PinyinParser::IsValidPinyin(L"zhuang"));
    CHECK("invalid pinyin 'xyz'", !PinyinParser::IsValidPinyin(L"xyz"));

    return results;
}
