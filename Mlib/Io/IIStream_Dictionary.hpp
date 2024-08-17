#pragma once
#include <Mlib/Source_Location.hpp>
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

class IIStreamDictionary {
public:
    virtual ~IIStreamDictionary() = default;
    virtual std::vector<std::string> names() const = 0;
    virtual std::unique_ptr<std::istream> read(
        const std::string& name,
        std::ios::openmode openmode,
        SourceLocation loc) = 0;
};

}
