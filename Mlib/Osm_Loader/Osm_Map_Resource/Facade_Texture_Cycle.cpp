#include "Facade_Texture_Cycle.hpp"
#include <Mlib/Misc/FPath.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture.hpp>
#include <stdexcept>

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
            throw std::runtime_error("Facade textures empty");
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
