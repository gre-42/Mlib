#include "Restore_Alpha_Channel.hpp"
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Images/Filters/Gaussian_Kernel.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Gen_Shader_Text.hpp>
#include <Mlib/OpenGL/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/OpenGL/Instance_Handles/Render_Guards.hpp>
#include <Mlib/OpenGL/Instance_Handles/Texture_Binder.hpp>
#include <Mlib/OpenGL/Instance_Handles/Texture_Layer_Properties.hpp>
#include <Mlib/OpenGL/Shader_Version_3_0.hpp>
#include <Mlib/OpenGL/Viewport_Guard.hpp>
#include <Mlib/Os/Env.hpp>

using namespace Mlib;

static GenShaderText filter_fragment_shader_text{[]()
    {
        std::stringstream sstr;
        sstr << std::scientific;
        sstr << SHADER_VER << FRAGMENT_PRECISION;
        sstr << "out vec4 FragColor;" << std::endl;
        sstr << std::endl;
        sstr << "in vec2 TexCoords;" << std::endl;
        sstr << std::endl;
        sstr << "uniform sampler2D texture_color;" << std::endl;
        sstr << "uniform sampler2D texture_alpha;" << std::endl;
        sstr << std::endl;
        sstr << "void main()" << std::endl;
        sstr << "{" << std::endl;
        sstr << "    FragColor = vec4(" << std::endl;
        sstr << "        texture(texture_color, TexCoords.st).rgb," << std::endl;
        sstr << "        texture(texture_alpha, TexCoords.st).r);" << std::endl;
        sstr << "}" << std::endl;
        if (getenv_default_bool("PRINT_SHADERS", false)) {
            linfo();
            linfo();
            linfo();
            linfo() << "Fragment";
            std::string line;
            for (size_t i = 1; std::getline(sstr, line); ++i) {
                linfo() << i << ": " << line;
            }
        }
        return sstr.str();
    }};

RestoreAlphaChannel::RestoreAlphaChannel() = default;

RestoreAlphaChannel::~RestoreAlphaChannel() = default;

void RestoreAlphaChannel::operator()(
    int width,
    int height,
    const std::shared_ptr<FrameBuffer>& alpha,
    const std::shared_ptr<FrameBuffer>& color_source,
    const std::shared_ptr<FrameBuffer>& color_dest)
{
    if (!rp_.allocated()) {
        rp_.allocate(simple_vertex_shader_text_, filter_fragment_shader_text());
        rp_.texture_color_location = rp_.get_uniform_location("texture_color");
        rp_.texture_alpha_location = rp_.get_uniform_location("texture_alpha");
    }
    notify_rendering(CURRENT_SOURCE_LOCATION);
    rp_.use();
    RenderToFrameBufferGuard rfg{ color_dest };
    ViewportGuard vg{ width, height };
    notify_rendering(CURRENT_SOURCE_LOCATION);

    TextureBinder tb;
    tb.bind(rp_.texture_alpha_location, *alpha->texture_color(), InterpolationPolicy::NEAREST_NEIGHBOR, TextureLayerProperties::NONE);
    tb.bind(rp_.texture_color_location, *color_source->texture_color(), InterpolationPolicy::NEAREST_NEIGHBOR, TextureLayerProperties::NONE);

    va().bind();
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));
}
