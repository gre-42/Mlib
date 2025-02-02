#pragma once
#include <Mlib/Source_Location.hpp>
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

struct StreamAndSize {
    std::unique_ptr<std::istream> stream;
    std::streamsize size;
};

class IIStreamDictionary {
public:
    virtual ~IIStreamDictionary() = default;
    virtual std::vector<std::string> names() const = 0;
    virtual StreamAndSize read(
        const std::string& name,
        std::ios::openmode openmode,
        SourceLocation loc) = 0;
};

}
