#include "Material.hpp"
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Material& Material::compute_color_mode() {
    for (auto& t : textures_color) {
        if (t.texture_descriptor.color.color_mode == ColorMode::UNDEFINED) {
            t.texture_descriptor.color.color_mode = (blend_mode == BlendMode::OFF)
                ? ColorMode::RGB
                : ColorMode::RGBA;
        }
    }
    return *this;
}

std::string Material::identifier() const {
    if (textures_color.size() > 0) {
        return "color: " + *textures_color.front().texture_descriptor.color.filename;
    } else {
        return "<no texture>";
    }
}

const BillboardAtlasInstance& Material::billboard_atlas_instance(
    BillboardId billboard_id,
    const std::string& name) const
{
    if (billboard_id >= billboard_atlas_instances.size()) {
        THROW_OR_ABORT(
            name + ": Billboard ID out of bounds in material \"" + identifier() + "\" (" +
            std::to_string(billboard_id) +
            " >= " +
            std::to_string(billboard_atlas_instances.size()) + ')');
    }
    return billboard_atlas_instances[billboard_id];
}

ScenePos Material::max_center_distance2(
    BillboardId billboard_id,
    const Morphology& morphology,
    const std::string& name) const
{
    return (billboard_id == BILLBOARD_ID_NONE)
        ? morphology.center_distances2(1)
        : billboard_atlas_instance(billboard_id, name).max_center_distance2;
}

ExternalRenderPassType Material::get_occluder_pass(
    BillboardId billboard_id,
    const std::string& name) const
{
    return (billboard_id == BILLBOARD_ID_NONE)
        ? occluder_pass
        : billboard_atlas_instance(billboard_id, name).occluder_pass;
}
