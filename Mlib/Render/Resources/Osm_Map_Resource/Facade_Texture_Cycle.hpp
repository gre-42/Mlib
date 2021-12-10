#pragma once
#include <Mlib/Render/Resources/Osm_Map_Resource/Resource_Cycle.hpp>

namespace Mlib {

struct FacadeTexture;
struct Building;

class FacadeTextureCycle: public ResourceCycle<FacadeTexture> {
public:
    FacadeTextureCycle(const std::vector<FacadeTexture>& names);
    const FacadeTexture& operator () (const Building& building);
};

}
