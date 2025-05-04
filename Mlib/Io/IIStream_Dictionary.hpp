#pragma once
#include <Mlib/Source_Location.hpp>
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

template <class T>
class VariableAndHash;

struct StreamAndSize {
    std::unique_ptr<std::istream> stream;
    std::streamsize size;
};

class IIStreamDictionary {
public:
    virtual ~IIStreamDictionary() = default;
    virtual std::vector<VariableAndHash<std::string>> names() const = 0;
    virtual StreamAndSize read(
        const VariableAndHash<std::string>& name,
        std::ios::openmode openmode,
        SourceLocation loc) = 0;
};

}
