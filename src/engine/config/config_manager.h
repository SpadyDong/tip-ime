#pragma once

#include <string>

namespace tip {

struct InputConfig {
    int candidateCount = 5;
    bool enableFuzzySound = true;
    bool enableJianpin = true;
    std::wstring switchLanguageKey = L"Shift";
};

class ConfigManager {
public:
    static ConfigManager& Instance();

    bool Load(const std::wstring& filePath);
    bool Save(const std::wstring& filePath) const;

    InputConfig& GetConfig();
    const InputConfig& GetConfig() const;

private:
    ConfigManager() = default;

    InputConfig config_;
};

} // namespace tip
