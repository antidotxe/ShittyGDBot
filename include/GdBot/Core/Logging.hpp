#pragma once

#include <fmt/format.h>

#include <string_view>
#include <utility>

namespace gdbot::logging {

enum class LogSeverity {
    kDebug,
    kInfo,
    kWarning,
    kError,
};

void logMessage(LogSeverity severity, std::string_view message);

template <typename... Args>
void debug(fmt::format_string<Args...> format, Args&&... args) {
    logMessage(LogSeverity::kDebug, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void info(fmt::format_string<Args...> format, Args&&... args) {
    logMessage(LogSeverity::kInfo, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void warning(fmt::format_string<Args...> format, Args&&... args) {
    logMessage(LogSeverity::kWarning, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void error(fmt::format_string<Args...> format, Args&&... args) {
    logMessage(LogSeverity::kError, fmt::format(format, std::forward<Args>(args)...));
}

}
