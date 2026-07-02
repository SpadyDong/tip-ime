#pragma once

#include <string>

namespace tip {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
};

class Logger {
public:
    static Logger& Instance();

    void SetLogLevel(LogLevel level);
    void SetLogFile(const std::wstring& filePath);

    void Debug(const std::wstring& message);
    void Info(const std::wstring& message);
    void Warning(const std::wstring& message);
    void Error(const std::wstring& message);

private:
    Logger() = default;
    void Log(LogLevel level, const std::wstring& message);

    LogLevel level_ = LogLevel::Info;
    std::wstring logFilePath_;
};

#define TIP_LOG_DEBUG(msg) tip::Logger::Instance().Debug(msg)
#define TIP_LOG_INFO(msg) tip::Logger::Instance().Info(msg)
#define TIP_LOG_WARNING(msg) tip::Logger::Instance().Warning(msg)
#define TIP_LOG_ERROR(msg) tip::Logger::Instance().Error(msg)

} // namespace tip
