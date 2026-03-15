#include <GdBot/Core/Logging.hpp>

#include <Geode/Geode.hpp>

namespace gdbot::logging {

using namespace geode::prelude;

void logMessage(LogSeverity severity, std::string_view message) {
    switch (severity) {
        case LogSeverity::kDebug:
            log::debug("{}", message);
            break;

        case LogSeverity::kInfo:
            log::info("{}", message);
            break;

        case LogSeverity::kWarning:
            log::warn("{}", message);
            break;

        case LogSeverity::kError:
            log::error("{}", message);
            break;
    }
}

}
