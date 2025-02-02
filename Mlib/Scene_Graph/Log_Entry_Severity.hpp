#pragma once
#include <string>

namespace Mlib {

enum class LogEntrySeverity {
    INFO,
    CRITICAL
};

LogEntrySeverity log_entry_severity_from_string(const std::string& s);

}
