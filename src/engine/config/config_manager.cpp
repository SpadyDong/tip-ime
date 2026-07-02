#include "config_manager.h"

#include "logger.h"

namespace tip {

ConfigManager& ConfigManager::Instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::Load(const std::wstring& filePath) {
    // TODO: implement INI/JSON config parsing
    TIP_LOG_INFO(L"Config loading from: " + filePath);
    return true;
}

bool ConfigManager::Save(const std::wstring& filePath) const {
    // TODO: implement INI/JSON config serialization
    TIP_LOG_INFO(L"Config saving to: " + filePath);
    return true;
}

InputConfig& ConfigManager::GetConfig() {
    return config_;
}

const InputConfig& ConfigManager::GetConfig() const {
    return config_;
}

} // namespace tip
