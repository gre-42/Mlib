#pragma once
#include <list>
#include <string>

namespace Mlib {

struct ReplacementParameterAndFilename;

class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;
    virtual std::list<ReplacementParameterAndFilename> try_load(const std::string& path) = 0;
};

}
