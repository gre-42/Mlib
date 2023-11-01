#include "Pointer_Image_Logic.hpp"
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rotation_2D.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>

using namespace Mlib;

PointerImageLogic::PointerImageLogic(
    RenderingResources& rendering_resources,
    const std::string& image_resource_name)
: FillWithTextureLogic{
    rendering_resources,
    image_resource_name,
    ResourceUpdateCycle::ONCE,
    ColorMode::RGBA,
    CullFaceMode::CULL,
    RenderTarget::CANVAS,
    nullptr }
{}

PointerImageLogic::~PointerImageLogic() = default;

void PointerImageLogic::render(
    const FixedArray<float, 2>& canvas_size,
    float angle,
    const FixedArray<float, 2>& center,
    const FixedArray<float, 2, 2, 2>& pointer_corners)
{
    LOG_FUNCTION("PointerImageLogic::render");
    // If angle = 0, the arrow points exactly upwards
    // => subtract pi/2 from angle.
    auto R = fixed_rotation_2d<float>(angle - float(M_PI) / 2.f);
    FixedArray<float, 2, 2, 2> pcr = dot(R, pointer_corners);
    float vertices[] = {
        // positions                                                                                                            // texCoords
        (center(0) + pcr(0u, 0u, 1u)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1u, 0u, 1u)) / canvas_size(1) * 2.f - 1.f, 0.0f, 1.0f,
        (center(0) + pcr(0u, 0u, 0u)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1u, 0u, 0u)) / canvas_size(1) * 2.f - 1.f, 0.0f, 0.0f,
        (center(0) + pcr(0u, 1u, 0u)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1u, 1u, 0u)) / canvas_size(1) * 2.f - 1.f, 1.0f, 0.0f,
    
        (center(0) + pcr(0u, 0u, 1u)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1u, 0u, 1u)) / canvas_size(1) * 2.f - 1.f, 0.0f, 1.0f,
        (center(0) + pcr(0u, 1u, 0u)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1u, 1u, 0u)) / canvas_size(1) * 2.f - 1.f, 1.0f, 0.0f,
        (center(0) + pcr(0u, 1u, 1u)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1u, 1u, 1u)) / canvas_size(1) * 2.f - 1.f, 1.0f, 1.0f
    };

    CHK(glBindBuffer(GL_ARRAY_BUFFER, va().vertex_buffer.handle()));
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    FillWithTextureLogic::render();
}
