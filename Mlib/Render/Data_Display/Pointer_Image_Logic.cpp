#include "Pointer_Image_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rotation_2D.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>

using namespace Mlib;

PointerImageLogic::PointerImageLogic(
    const std::shared_ptr<ITextureHandle>& texture)
: FillWithTextureLogic{
    texture,
    CullFaceMode::CULL,
    ContinuousBlendMode::ALPHA,
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
        // positions                                                                                                            // tex_coords
        (center(0) + pcr(0, 0, 1)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1, 0, 1)) / canvas_size(1) * 2.f - 1.f, 0.0f, 1.0f,
        (center(0) + pcr(0, 0, 0)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1, 0, 0)) / canvas_size(1) * 2.f - 1.f, 0.0f, 0.0f,
        (center(0) + pcr(0, 1, 0)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1, 1, 0)) / canvas_size(1) * 2.f - 1.f, 1.0f, 0.0f,
    
        (center(0) + pcr(0, 0, 1)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1, 0, 1)) / canvas_size(1) * 2.f - 1.f, 0.0f, 1.0f,
        (center(0) + pcr(0, 1, 0)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1, 1, 0)) / canvas_size(1) * 2.f - 1.f, 1.0f, 0.0f,
        (center(0) + pcr(0, 1, 1)) / canvas_size(0) * 2.f - 1.f, (center(1) + pcr(1, 1, 1)) / canvas_size(1) * 2.f - 1.f, 1.0f, 1.0f
    };

    va();  // Initialize if necessary
    vertices_.bind();
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    FillWithTextureLogic::render(ClearMode::OFF);
}
