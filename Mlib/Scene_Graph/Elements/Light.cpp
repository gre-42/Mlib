#include "Light.hpp"
#include <Mlib/Geometry/Material/Render_Pass.hpp>

using namespace Mlib;

bool Light::emits_colors() const {
    return (shadow_render_pass == ExternalRenderPassType::NONE) ||
        any(shadow_render_pass & ExternalRenderPassType::LIGHTMAP_EMITS_COLORS_MASK);
}
