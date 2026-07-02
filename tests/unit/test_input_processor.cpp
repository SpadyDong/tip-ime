#include "engine/input_processor.h"

#include <vector>

using namespace tip;

struct TestResult {
    const char* name;
    bool passed;
};

#define CHECK(name, condition) results.push_back({ name, (condition) })

std::vector<TestResult> RunInputProcessorTests() {
    std::vector<TestResult> results;

    InputProcessor processor;
    processor.Initialize();

    CHECK("initial raw pinyin is empty", processor.GetRawPinyin().empty());

    processor.AppendPinyinChar(L'z');
    processor.AppendPinyinChar(L'h');
    processor.AppendPinyinChar(L'o');
    processor.AppendPinyinChar(L'n');
    processor.AppendPinyinChar(L'g');
    CHECK("raw pinyin accumulates", processor.GetRawPinyin() == L"zhong");

    processor.Backspace();
    CHECK("backspace removes last char", processor.GetRawPinyin() == L"zhon");

    auto candidates = processor.GetCandidates();
    CHECK("candidates not empty when raw pinyin exists", !candidates.empty());

    processor.Clear();
    CHECK("clear resets raw pinyin", processor.GetRawPinyin().empty());
    CHECK("clear resets committed text", processor.GetCommittedText().empty());

    // Pagination tests.
    processor.SetPageSize(2);
    CHECK("page size is set", processor.GetPageSize() == 2);

    processor.AppendPinyinChar(L'z');
    processor.AppendPinyinChar(L'h');
    processor.AppendPinyinChar(L'o');
    processor.AppendPinyinChar(L'n');
    processor.AppendPinyinChar(L'g');
    CHECK("raw pinyin is zhong", processor.GetRawPinyin() == L"zhong");

    auto paged = processor.GetPagedCandidates();
    CHECK("paged candidates respect page size", paged.size() <= 2);

    size_t totalPages = processor.GetTotalPages();
    CHECK("total pages is positive", totalPages > 0);
    CHECK("current page is 0 initially", processor.GetCurrentPage() == 0);

    if (totalPages > 1) {
        CHECK("page down succeeds when more pages exist", processor.PageDown());
        CHECK("current page increments", processor.GetCurrentPage() == 1);
        CHECK("page up succeeds", processor.PageUp());
        CHECK("current page decrements", processor.GetCurrentPage() == 0);
    }

    CHECK("page up fails on first page", !processor.PageUp());

    processor.Clear();
    processor.SetPageSize(5);

    processor.Shutdown();

    return results;
}
