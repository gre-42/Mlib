#pragma once
#include <ios>
#include <iosfwd>
#include <string>

namespace Mlib {

class OstreamSizeLogger {
public:
    OstreamSizeLogger(std::ostream& ostr, std::string message);
    ~OstreamSizeLogger();
private:
    std::ostream& ostr_;
    std::string message_;
    std::streampos streampos_;
};

}
