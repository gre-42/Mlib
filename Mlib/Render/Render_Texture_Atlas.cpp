#include "Render_Texture_Atlas.hpp"
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

void Mlib::render_texture_atlas(
    RenderingResources& rendering_resources,
    const std::vector<AutoAtlasTileDescriptor>& tiles,
    float scale_width,
    float scale_height)
{
    FillWithTextureLogic logic{
        rendering_resources,
        { .filename = "" },
        ResourceUpdateCycle::ONCE,
        CullFaceMode::CULL,
        AlphaChannelRole::NO_BLEND };
    for (const auto& tile : tiles) {
        ViewportGuard vg{
            (float)tile.left * scale_width,
            (float)tile.bottom * scale_height,
            (float)tile.width * scale_width,
            (float)tile.height * scale_height};
        logic.set_image_resource_name({
            .filename = tile.filename,
            .color_mode = ColorMode::RGBA,
            .mipmap_mode = MipmapMode::WITH_MIPMAPS,});
        logic.render();
    }
}
