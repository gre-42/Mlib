#include "Instances_Are_Visible.hpp"
#include <Mlib/Geometry/Material.hpp>

using namespace Mlib;

bool Mlib::instances_are_visible(
    const Material& m,
    ExternalRenderPassType external_render_pass)
{
    if (any(external_render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        auto rp = external_render_pass & ~ExternalRenderPassType::PRELOAD_MASK;
        return (m.occluder_pass & rp) == rp;
    } else {
        return true;
    }
}
