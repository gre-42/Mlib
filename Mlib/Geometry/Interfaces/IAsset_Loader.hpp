#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <list>
#include <string>

namespace Mlib {

struct ReplacementParameterAndFilename;

class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;
    virtual std::list<ReplacementParameterAndFilename> try_load(const Utf8Path& path) = 0;
};

}
