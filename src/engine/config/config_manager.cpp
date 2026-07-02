#include "config_manager.h"

#include <fstream>
#include <sstream>

#include "logger.h"
#include "string_utils.h"

namespace tip {

namespace {

std::wstring TrimWide(const std::wstring& input) {
    size_t start = 0;
    while (start < input.size() && (input[start] == L' ' || input[start] == L'\t' ||
                                     input[start] == L'\r')) {
        ++start;
    }
    if (start >= input.size()) {
        return L"";
    }

    size_t end = input.size() - 1;
    while (end > start && (input[end] == L' ' || input[end] == L'\t' ||
                            input[end] == L'\r')) {
        --end;
    }
    return input.substr(start, end - start + 1);
}

bool ParseBool(const std::wstring& value) {
    std::wstring lower = ToLowerWide(value);
    return lower == L"1" || lower == L"true" || lower == L"yes" || lower == L"on";
}

} // namespace

ConfigManager& ConfigManager::Instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::Load(const std::wstring& filePath) {
    std::string utf8Path = WideToUtf8(filePath);
    std::ifstream file(utf8Path);
    if (!file.is_open()) {
        TIP_LOG_WARNING(L"Failed to open config file: " + filePath);
        return false;
    }

    std::string line;
    std::wstring currentSection;
    while (std::getline(file, line)) {
        std::wstring wideLine = Utf8ToWide(line);
        wideLine = TrimWide(wideLine);
        if (wideLine.empty() || wideLine.front() == L'#' || wideLine.front() == L';') {
            continue;
        }

        if (wideLine.front() == L'[' && wideLine.back() == L']') {
            currentSection = ToLowerWide(wideLine.substr(1, wideLine.size() - 2));
            continue;
        }

        size_t equalPos = wideLine.find(L'=');
        if (equalPos == std::wstring::npos) {
            continue;
        }

        std::wstring key = TrimWide(wideLine.substr(0, equalPos));
        std::wstring value = TrimWide(wideLine.substr(equalPos + 1));
        if (key.empty()) {
            continue;
        }

        key = ToLowerWide(key);

        if (currentSection == L"input") {
            if (key == L"candidate_count") {
                try {
                    config_.candidateCount = std::stoi(value);
                    if (config_.candidateCount < 1) {
                        config_.candidateCount = 1;
                    } else if (config_.candidateCount > 9) {
                        config_.candidateCount = 9;
                    }
                } catch (...) {
                    config_.candidateCount = 5;
                }
            } else if (key == L"enable_fuzzy_sound") {
                config_.enableFuzzySound = ParseBool(value);
            } else if (key == L"enable_jianpin") {
                config_.enableJianpin = ParseBool(value);
            } else if (key == L"switch_language_key") {
                config_.switchLanguageKey = value;
            }
        } else if (currentSection == L"ui") {
            if (key == L"skin") {
                config_.skin = value;
            } else if (key == L"high_dpi") {
                config_.highDpi = ParseBool(value);
            }
        }
    }

    TIP_LOG_INFO(L"Config loaded from: " + filePath);
    return true;
}

bool ConfigManager::Save(const std::wstring& filePath) const {
    std::string utf8Path = WideToUtf8(filePath);
    std::ofstream file(utf8Path);
    if (!file.is_open()) {
        TIP_LOG_WARNING(L"Failed to open config file for writing: " + filePath);
        return false;
    }

    file << "# TIP Input Method Configuration\n";
    file << "\n";
    file << "[input]\n";
    file << "candidate_count=" << config_.candidateCount << "\n";
    file << "enable_fuzzy_sound=" << (config_.enableFuzzySound ? 1 : 0) << "\n";
    file << "enable_jianpin=" << (config_.enableJianpin ? 1 : 0) << "\n";
    file << "switch_language_key=" << WideToUtf8(config_.switchLanguageKey) << "\n";
    file << "\n";
    file << "[ui]\n";
    file << "skin=" << WideToUtf8(config_.skin) << "\n";
    file << "high_dpi=" << (config_.highDpi ? 1 : 0) << "\n";

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
