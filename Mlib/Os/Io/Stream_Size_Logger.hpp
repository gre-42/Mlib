#pragma once
#include <ios>
#include <iosfwd>
#include <string>

namespace Mlib {

class StreamSizeLogger {
public:
    StreamSizeLogger(std::istream& istr, std::string message);
    ~StreamSizeLogger();
private:
    std::istream& istr_;
    std::string message_;
    std::streampos streampos_;
};

}
