#include "Log_Entry_Severity.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

LogEntrySeverity Mlib::log_entry_severity_from_string(const std::string& s) {
    if (s == "info") {
        return LogEntrySeverity::INFO;
    } else if (s == "critical") {
        return LogEntrySeverity::CRITICAL;
    } else {
        THROW_OR_ABORT("Unknown log entry severity");
    }
}
