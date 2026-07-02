#include "engine/dictionary/dictionary.h"

#include <vector>

using namespace tip;

struct TestResult {
    const char* name;
    bool passed;
};

#define CHECK(name, condition) results.push_back({ name, (condition) })

std::vector<TestResult> RunDictionaryTests() {
    std::vector<TestResult> results;

    Dictionary dict;
    dict.AddEntry(L"nihao", L"你好", 100);
    dict.AddEntry(L"zhongguo", L"中国", 90);

    CHECK("size after add", dict.Size() == 2);

    auto matches = dict.Query(L"nihao");
    CHECK("query exact pinyin", matches.size() == 1 && matches[0].text == L"你好");

    CHECK("has existing entry", dict.HasEntry(L"nihao", L"你好"));
    CHECK("does not have missing entry", !dict.HasEntry(L"nihao", L"再见"));

    auto all = dict.GetAllEntries();
    CHECK("get all entries", all.size() == 2);

    bool removed = dict.RemoveEntry(L"nihao", L"你好");
    CHECK("remove existing entry", removed);
    CHECK("size after remove", dict.Size() == 1);

    return results;
}
