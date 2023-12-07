#include "Facade_Texture_Cycle.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

FacadeTextureCycle::FacadeTextureCycle(const std::vector<FacadeTexture>& names)
: ResourceCycle{ names }
{
    for (const auto& n : names) {
        if (n.selector.empty()) {
            continue;
        }
        if (!ftm_.insert({n.selector, &n}).second) {
            THROW_OR_ABORT("Found duplicate facade style \"" + n.selector + '"');
        }
    }
}

const FacadeTexture& FacadeTextureCycle::operator () (
    const std::string& style,
    float building_top)
{
    if (!style.empty()) {
        auto ft = from_style(style);
        if (ft == nullptr) {
            lwarn() << "Unknown material: \"" + style + '"';
            return from_building_top(building_top);
        } else {
            return *ft;
        }
    } else {
        if (empty()) {
            THROW_OR_ABORT("Facade textures empty");
        }
        return from_building_top(building_top);
    }
}

const FacadeTexture& FacadeTextureCycle::from_building_top(float building_top) {
    ResourceCycle& rc = *this;
    return rc([&building_top](const FacadeTexture& tex){
        return
            (building_top >= tex.min_height) &&
            (building_top <= tex.max_height);
    });
}

const FacadeTexture* FacadeTextureCycle::from_style(const std::string& style) {
    auto it = ftm_.find(style);
    return it == ftm_.end()
        ? nullptr
        : it->second;
}
