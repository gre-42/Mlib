#pragma once
#include <iosfwd>

namespace Mlib {

class BaseLog {
public:
    virtual void log(const std::string& message) = 0;
    virtual void get_messages(std::ostream& ostr, size_t nentries) const = 0;
};

}
