#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class LogEntrySeverity {
    INFO,
    CRITICAL
};

LogEntrySeverity log_entry_severity_from_string(const std::string& s) {
    if (s == "info") {
        return LogEntrySeverity::INFO;
    } else if (s == "critical") {
        return LogEntrySeverity::CRITICAL;
    } else {
        throw std::runtime_error("Unknown log entry severity");
    }
}

}
