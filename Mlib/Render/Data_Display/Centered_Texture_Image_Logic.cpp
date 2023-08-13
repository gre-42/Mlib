#include "Centered_Texture_Image_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rotation_2D.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>

using namespace Mlib;

CenteredTextureImageLogic::CenteredTextureImageLogic(
    const std::string& image_resource_name,
    ColorMode color_mode)
: FillWithTextureLogic{ image_resource_name, ResourceUpdateCycle::ONCE, color_mode, CullFaceMode::NO_CULL }
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
        // positions                                                         // texCoords
        -pcr(0u, 0u, 1u) / canvas_size(0), pcr(1u, 0u, 1u) / canvas_size(1), 0.0f, 0.0f,
        -pcr(0u, 0u, 0u) / canvas_size(0), pcr(1u, 0u, 0u) / canvas_size(1), 0.0f, 1.0f,
        -pcr(0u, 1u, 0u) / canvas_size(0), pcr(1u, 1u, 0u) / canvas_size(1), 1.0f, 1.0f,

        -pcr(0u, 0u, 1u) / canvas_size(0), pcr(1u, 0u, 1u) / canvas_size(1), 0.0f, 0.0f,
        -pcr(0u, 1u, 0u) / canvas_size(0), pcr(1u, 1u, 0u) / canvas_size(1), 1.0f, 1.0f,
        -pcr(0u, 1u, 1u) / canvas_size(0), pcr(1u, 1u, 1u) / canvas_size(1), 1.0f, 0.0f
    };

    CHK(glBindBuffer(GL_ARRAY_BUFFER, va().vertex_buffer));
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    FillWithTextureLogic::render();
}
