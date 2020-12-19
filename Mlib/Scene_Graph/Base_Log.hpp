#pragma once
#include <iosfwd>

namespace Mlib {

enum class LogEntrySeverity;

class BaseLog {
public:
    virtual void log(const std::string& message, LogEntrySeverity severity) = 0;
    virtual void get_messages(std::ostream& ostr, size_t nentries, LogEntrySeverity severity) const = 0;
};

}
