#include "logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace tip {

namespace {

std::mutex g_logMutex;

std::wstring LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return L"DEBUG";
        case LogLevel::Info: return L"INFO";
        case LogLevel::Warning: return L"WARNING";
        case LogLevel::Error: return L"ERROR";
    }
    return L"UNKNOWN";
}

} // namespace

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

void Logger::SetLogLevel(LogLevel level) {
    level_ = level;
}

void Logger::SetLogFile(const std::wstring& filePath) {
    logFilePath_ = filePath;
}

void Logger::Debug(const std::wstring& message) {
    Log(LogLevel::Debug, message);
}

void Logger::Info(const std::wstring& message) {
    Log(LogLevel::Info, message);
}

void Logger::Warning(const std::wstring& message) {
    Log(LogLevel::Warning, message);
}

void Logger::Error(const std::wstring& message) {
    Log(LogLevel::Error, message);
}

void Logger::Log(LogLevel level, const std::wstring& message) {
    if (level < level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_logMutex);

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::wstringstream ss;
    ss << std::put_time(&localTime, L"%Y-%m-%d %H:%M:%S")
       << L" [" << LevelToString(level) << L"] "
       << message;

    // TODO: write to log file if logFilePath_ is set
    std::wcout << ss.str() << std::endl;
}

} // namespace tip
