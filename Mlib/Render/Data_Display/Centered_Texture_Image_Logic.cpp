#include "Centered_Texture_Image_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rotation_2D.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>

using namespace Mlib;

CenteredTextureImageLogic::CenteredTextureImageLogic(
    const std::shared_ptr<ITextureHandle>& texture,
    ContinuousBlendMode blend_mode)
    : FillWithTextureLogic{
        texture,
        CullFaceMode::CULL,
        blend_mode,
        nullptr }
{}

CenteredTextureImageLogic::~CenteredTextureImageLogic() = default;

void CenteredTextureImageLogic::render(
    const FixedArray<float, 2>& canvas_size,
    float angle,
    const FixedArray<float, 2, 2, 2>& pointer_corners)
{
    LOG_FUNCTION("CenteredTextureImageLogic::render");
    auto R = fixed_rotation_2d<float>(angle);
    FixedArray<float, 2, 2, 2> pcr = dot(R, pointer_corners);
    float vertices[] = {
        // positions                                                         // tex_coords
        pcr(0, 0, 0) / canvas_size(0), -pcr(1, 0, 0) / canvas_size(1), 0.0f, 1.0f,
        pcr(0, 0, 1) / canvas_size(0), -pcr(1, 0, 1) / canvas_size(1), 0.0f, 0.0f,
        pcr(0, 1, 1) / canvas_size(0), -pcr(1, 1, 1) / canvas_size(1), 1.0f, 0.0f,

        pcr(0, 0, 0) / canvas_size(0), -pcr(1, 0, 0) / canvas_size(1), 0.0f, 1.0f,
        pcr(0, 1, 1) / canvas_size(0), -pcr(1, 1, 1) / canvas_size(1), 1.0f, 0.0f,
        pcr(0, 1, 0) / canvas_size(0), -pcr(1, 1, 0) / canvas_size(1), 1.0f, 1.0f
    };

    va();  // Initialize if necessary
    vertices_.bind();
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    FillWithTextureLogic::render(ClearMode::OFF);
}
