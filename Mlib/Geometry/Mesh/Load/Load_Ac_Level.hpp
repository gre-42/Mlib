#pragma once
#include <Mlib/Geometry/Interfaces/IAsset_Loader.hpp>

namespace Mlib {

struct ReplacementParameterAndFilename;

class LoadAcLevel: public IAssetLoader {
public:
    explicit LoadAcLevel(std::string script_filename);
    virtual ~LoadAcLevel() override;
    virtual std::list<ReplacementParameterAndFilename> try_load(const std::string& path) override;
private:
    std::string script_filename_;
};

}
