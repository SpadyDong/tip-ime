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

    processor.Shutdown();

    return results;
}
