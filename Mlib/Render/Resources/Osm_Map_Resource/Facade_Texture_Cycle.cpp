#include "Facade_Texture_Cycle.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Facade_Texture.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>

using namespace Mlib;

FacadeTextureCycle::FacadeTextureCycle(const std::vector<FacadeTexture>& names)
: ResourceCycle{ names }
{}

const FacadeTexture& FacadeTextureCycle::operator () (const Building& building) {
    ResourceCycle& rc = *this;
    return rc([&building](const FacadeTexture& tex){
        return
            (building.levels.back().top >= tex.min_height) &&
            (building.levels.back().top <= tex.max_height);
    });
}
