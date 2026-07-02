#include "engine/state/input_state.h"

#include <vector>

using namespace tip;

struct TestResult {
    const char* name;
    bool passed;
};

#define CHECK(name, condition) results.push_back({ name, (condition) })

std::vector<TestResult> RunInputStateTests() {
    std::vector<TestResult> results;

    InputState state;
    CHECK("default language is Chinese", state.languageMode == LanguageMode::Chinese);

    state.ToggleLanguage();
    CHECK("toggle language to English", state.languageMode == LanguageMode::English);

    state.ToggleCharWidth();
    CHECK("toggle char width to Full", state.charWidthMode == CharWidthMode::Full);

    state.TogglePunctuation();
    CHECK("toggle punctuation to English", state.punctuationMode == PunctuationMode::English);

    return results;
}
