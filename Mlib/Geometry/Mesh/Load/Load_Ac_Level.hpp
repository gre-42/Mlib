#pragma once
#include <Mlib/Geometry/Interfaces/IAsset_Loader.hpp>

namespace Mlib {

struct ReplacementParameterAndFilename;

class LoadAcLevel: public IAssetLoader {
public:
    explicit LoadAcLevel(Utf8Path script_filename);
    virtual ~LoadAcLevel() override;
    virtual std::list<ReplacementParameterAndFilename> try_load(const Utf8Path& path) override;
private:
    Utf8Path script_filename_;
};

}
