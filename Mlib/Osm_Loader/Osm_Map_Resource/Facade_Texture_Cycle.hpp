#pragma once
#include <Mlib/Osm_Loader/Osm_Map_Resource/Resource_Cycle.hpp>
#include <map>

namespace Mlib {

struct FacadeTexture;
struct Building;

class FacadeTextureCycle: public ResourceCycle<FacadeTexture> {
public:
    FacadeTextureCycle(const std::vector<FacadeTexture>& names);
    const FacadeTexture& operator () (float building_top);
    const FacadeTexture* operator () (const std::string& style);
private:
    std::map<std::string, const FacadeTexture*> ftm_;
};

}
