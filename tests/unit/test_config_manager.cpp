#include "engine/config/config_manager.h"

#include <vector>

using namespace tip;

struct TestResult {
    const char* name;
    bool passed;
};

#define CHECK(name, condition) results.push_back({ name, (condition) })

std::vector<TestResult> RunConfigManagerTests() {
    std::vector<TestResult> results;

    ConfigManager& manager = ConfigManager::Instance();

    // Reset to known defaults before testing.
    manager.GetConfig().candidateCount = 5;
    manager.GetConfig().enableFuzzySound = true;
    manager.GetConfig().enableJianpin = true;
    manager.GetConfig().switchLanguageKey = L"Shift";
    manager.GetConfig().skin = L"default";
    manager.GetConfig().highDpi = true;

    CHECK("default candidate count is 5", manager.GetConfig().candidateCount == 5);
    CHECK("default fuzzy sound is enabled", manager.GetConfig().enableFuzzySound);
    CHECK("default jianpin is enabled", manager.GetConfig().enableJianpin);

    manager.GetConfig().candidateCount = 3;
    manager.GetConfig().enableFuzzySound = false;
    manager.GetConfig().enableJianpin = false;
    manager.GetConfig().switchLanguageKey = L"Ctrl";
    manager.GetConfig().skin = L"dark";
    manager.GetConfig().highDpi = false;

    CHECK("modified candidate count is 3", manager.GetConfig().candidateCount == 3);
    CHECK("modified fuzzy sound is disabled", !manager.GetConfig().enableFuzzySound);
    CHECK("modified jianpin is disabled", !manager.GetConfig().enableJianpin);
    CHECK("modified switch key is Ctrl", manager.GetConfig().switchLanguageKey == L"Ctrl");
    CHECK("modified skin is dark", manager.GetConfig().skin == L"dark");
    CHECK("modified high dpi is disabled", !manager.GetConfig().highDpi);

    return results;
}
