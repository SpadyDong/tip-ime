#include <iostream>
#include <vector>

// Minimal test framework for the skeleton stage.
struct TestResult {
    const char* name;
    bool passed;
};

extern std::vector<TestResult> RunPinyinParserTests();
extern std::vector<TestResult> RunTrieIndexTests();
extern std::vector<TestResult> RunInputStateTests();
extern std::vector<TestResult> RunInputProcessorTests();
extern std::vector<TestResult> RunStringUtilsTests();

int main() {
    std::vector<TestResult> allResults;

    auto append = [&allResults](const std::vector<TestResult>& results) {
        allResults.insert(allResults.end(), results.begin(), results.end());
    };

    append(RunPinyinParserTests());
    append(RunTrieIndexTests());
    append(RunInputStateTests());
    append(RunInputProcessorTests());
    append(RunStringUtilsTests());

    int passed = 0;
    int failed = 0;
    for (const auto& result : allResults) {
        if (result.passed) {
            ++passed;
            std::cout << "[PASS] " << result.name << std::endl;
        } else {
            ++failed;
            std::cout << "[FAIL] " << result.name << std::endl;
        }
    }

    std::cout << "\nTotal: " << (passed + failed)
              << ", Passed: " << passed
              << ", Failed: " << failed << std::endl;

    return failed == 0 ? 0 : 1;
}
