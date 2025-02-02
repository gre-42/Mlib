#include "Render_Texture_Atlas.hpp"
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

void Mlib::render_texture_atlas(
    const RenderingResources& rendering_resources,
    const std::vector<AutoAtlasTileDescriptor>& tiles,
    float scale_width,
    float scale_height)
{
    FillWithTextureLogic logic{
        nullptr,
        CullFaceMode::CULL,
        ContinuousBlendMode::NONE };
    for (const auto& tile : tiles) {
        ViewportGuard vg{
            (float)tile.left * scale_width,
            (float)tile.bottom * scale_height,
            (float)tile.width * scale_width,
            (float)tile.height * scale_height};
        logic.set_image_resource_name(rendering_resources.get_texture(tile.name));
        logic.render(ClearMode::OFF);
    }
}
