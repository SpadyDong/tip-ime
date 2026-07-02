#include "engine/dictionary/trie_index.h"

#include <vector>

using namespace tip;

struct TestResult {
    const char* name;
    bool passed;
};

#define CHECK(name, condition) results.push_back({ name, (condition) })

std::vector<TestResult> RunTrieIndexTests() {
    std::vector<TestResult> results;

    TrieIndex index;
    index.Insert(L"zhongguo", { L"中国", L"zhongguo", 100 });
    index.Insert(L"zhongguo", { L"中国", L"zhongguo", 100 });
    index.Insert(L"beijing", { L"北京", L"beijing", 90 });

    auto r1 = index.Search(L"zhong");
    CHECK("search 'zhong' returns 1 entry", r1.size() == 1 && r1[0].text == L"中国");

    auto r2 = index.Search(L"beijing");
    CHECK("search 'beijing' returns 1 entry", r2.size() == 1 && r2[0].text == L"北京");

    auto r3 = index.Search(L"none");
    CHECK("search non-existing prefix returns empty", r3.empty());

    bool removed = index.Remove(L"zhongguo", L"中国");
    CHECK("remove existing entry returns true", removed);
    auto r4 = index.Search(L"zhong");
    CHECK("after remove, search returns empty", r4.empty());

    return results;
}
