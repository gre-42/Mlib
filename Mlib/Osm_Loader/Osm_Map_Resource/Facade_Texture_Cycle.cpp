#include "Facade_Texture_Cycle.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture.hpp>

using namespace Mlib;

FacadeTextureCycle::FacadeTextureCycle(const std::vector<FacadeTexture>& names)
: ResourceCycle{ names }
{
    for (const auto& n : names) {
        if (n.selector.empty()) {
            continue;
        }
        if (!ftm_.insert({n.selector, &n}).second) {
            throw std::runtime_error("Found duplicate facade style \"" + n.selector + '"');
        }
    }
}

const FacadeTexture& FacadeTextureCycle::operator () (const Building& building) {
    ResourceCycle& rc = *this;
    return rc([&building](const FacadeTexture& tex){
        return
            (building.levels.back().top >= tex.min_height) &&
            (building.levels.back().top <= tex.max_height);
    });
}

const FacadeTexture* FacadeTextureCycle::operator () (const std::string& style) {
    auto it = ftm_.find(style);
    return it == ftm_.end()
        ? nullptr
        : it->second;
}
