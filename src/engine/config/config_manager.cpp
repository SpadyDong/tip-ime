#include "config_manager.h"

#include <fstream>
#include <sstream>

#include "logger.h"
#include "utils/string_utils.h"

namespace tip {

namespace {

std::wstring TrimWide(const std::wstring& s) {
    size_t start = 0;
    while (start < s.size() && (s[start] == L' ' || s[start] == L'\t' || s[start] == L'\r')) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && (s[end - 1] == L' ' || s[end - 1] == L'\t' || s[end - 1] == L'\r')) {
        --end;
    }
    return s.substr(start, end - start);
}

bool ParseBool(const std::wstring& value) {
    if (value.empty()) {
        return false;
    }
    return value == L"1" || value == L"true" || value == L"True" || value == L"TRUE";
}

} // namespace

ConfigManager& ConfigManager::Instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::Load(const std::wstring& filePath) {
#ifdef _WIN32
    std::ifstream file(filePath);
#else
    std::ifstream file(WideToUtf8(filePath));
#endif
    if (!file.is_open()) {
        TIP_LOG_INFO(L"Config loading failed: " + filePath);
        return false;
    }

    std::string lineUtf8;
    std::wstring section;
    while (std::getline(file, lineUtf8)) {
        std::wstring line = TrimWide(Utf8ToWide(lineUtf8));
        if (line.empty() || line[0] == L'#' || line[0] == L';') {
            continue;
        }
        if (line.front() == L'[' && line.back() == L']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) {
            continue;
        }
        std::wstring key = TrimWide(line.substr(0, eq));
        std::wstring value = TrimWide(line.substr(eq + 1));

        if (section == L"input") {
            if (key == L"candidate_count") {
                config_.candidateCount = std::max(1, std::stoi(value));
            } else if (key == L"enable_fuzzy_sound") {
                config_.enableFuzzySound = ParseBool(value);
            } else if (key == L"enable_jianpin") {
                config_.enableJianpin = ParseBool(value);
            } else if (key == L"switch_language_key") {
                config_.switchLanguageKey = value;
            } else if (key == L"corner_radius") {
                config_.cornerRadius = std::max(0, std::stoi(value));
            }
        }
    }

    TIP_LOG_INFO(L"Config loaded from: " + filePath);
    return true;
}

bool ConfigManager::Save(const std::wstring& filePath) const {
#ifdef _WIN32
    std::ofstream file(filePath);
#else
    std::ofstream file(WideToUtf8(filePath));
#endif
    if (!file.is_open()) {
        TIP_LOG_INFO(L"Config saving failed: " + filePath);
        return false;
    }

    file << WideToUtf8(L"[input]\n");
    file << WideToUtf8(L"candidate_count=" + std::to_wstring(config_.candidateCount) + L"\n");
    file << WideToUtf8(L"enable_fuzzy_sound=" + std::wstring(config_.enableFuzzySound ? L"1" : L"0") + L"\n");
    file << WideToUtf8(L"enable_jianpin=" + std::wstring(config_.enableJianpin ? L"1" : L"0") + L"\n");
    file << WideToUtf8(L"switch_language_key=" + config_.switchLanguageKey + L"\n");
    file << WideToUtf8(L"corner_radius=" + std::to_wstring(config_.cornerRadius) + L"\n");
    file << WideToUtf8(L"\n[ui]\n");
    file << WideToUtf8(L"skin=default\n");
    file << WideToUtf8(L"high_dpi=1\n");

    TIP_LOG_INFO(L"Config saved to: " + filePath);
    return true;
}

InputConfig& ConfigManager::GetConfig() {
    return config_;
}

const InputConfig& ConfigManager::GetConfig() const {
    return config_;
}

} // namespace tip
