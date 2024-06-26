#include "Light.hpp"
#include <Mlib/Scene_Graph/Render_Pass.hpp>

using namespace Mlib;

bool Light::light_emits_colors() const {
	return (shadow_render_pass == ExternalRenderPassType::NONE) ||
		any(shadow_render_pass & ExternalRenderPassType::LIGHTMAP_EMITS_COLORS_MASK);
}
