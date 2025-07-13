#include "Lowpass.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Images/Filters/Gaussian_Kernel.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Instance_Handles/Texture_Binder.hpp>
#include <Mlib/Render/Instance_Handles/Texture_Layer_Properties.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

static GenShaderText filter_fragment_shader_text{[](
    const std::variant<BoxParameters, NormalParameters>& params,
    LowpassFlavor flavor,
    size_t axis)
    {
        std::stringstream sstr;
        sstr << std::scientific;
        sstr << SHADER_VER << FRAGMENT_PRECISION;
        sstr << "out vec4 FragColor;" << std::endl;
        sstr << std::endl;
        sstr << "in vec2 TexCoords;" << std::endl;
        sstr << std::endl;
        sstr << "uniform sampler2D texture_color;" << std::endl;
        sstr << "uniform float offset;" << std::endl;
        sstr << std::endl;
        sstr << "void main()" << std::endl;
        sstr << "{" << std::endl;
        sstr << "    vec3 center = texture(texture_color, TexCoords.st).rgb;" << std::endl;
        std::visit(overloads{
            [&](const BoxParameters&){
                if (axis == 0) {
                    sstr << "    vec3 color =" << std::endl;
                    sstr << "          center" << std::endl;
                    sstr << "        + texture(texture_color, TexCoords.st + vec2(offset, 0.0)).rgb" << std::endl;
                    sstr << "        + texture(texture_color, TexCoords.st - vec2(offset, 0.0)).rgb;" << std::endl;
                } else if (axis == 1) {
                    sstr << "    vec3 color =" << std::endl;
                    sstr << "          center" << std::endl;
                    sstr << "        + texture(texture_color, TexCoords.st + vec2(0.0, offset)).rgb" << std::endl;
                    sstr << "        + texture(texture_color, TexCoords.st - vec2(0.0, offset)).rgb;" << std::endl;
                } else {
                    THROW_OR_ABORT("Unknown texture axis");
                }
                sstr << "    FragColor = vec4(color / 3.0, 1.0);" << std::endl;
            },
            [&](const NormalParameters& params){
                auto weights = gaussian_kernel(params.stddev, 3.5f);
                int cdist = integral_cast<int>(weights.length() / 2);
                if (axis == 0) {
                    sstr << "    FragColor = vec4(0.0, 0.0, 0.0, 1.0);" << std::endl;
                    for (const auto& [i, w] : tenumerate<int>(weights.flat_iterable())) {
                        sstr <<
                            "    FragColor.rgb += " << w <<
                            " * texture(texture_color, TexCoords.st + vec2(" <<
                            integral_to_float<float>(i - cdist) << " * offset, 0.0)).rgb;" << std::endl;
                    }
                } else if (axis == 1) {
                    sstr << "    FragColor = vec4(0.0, 0.0, 0.0, 1.0);" << std::endl;
                    for (const auto& [i, w] : tenumerate<int>(weights.flat_iterable())) {
                        sstr <<
                            "    FragColor.rgb += " << w <<
                            " * texture(texture_color, TexCoords.st + vec2(0.0, " <<
                            integral_to_float<float>(i - cdist) << " * offset)).rgb;" << std::endl;
                    }
                } else {
                    THROW_OR_ABORT("Unknown texture axis");
                }
            },
            }, params);
        if (flavor == LowpassFlavor::MAX) {
            sstr << "    FragColor.rgb = max(FragColor.rgb, center);" << std::endl;
        }
        if (flavor == LowpassFlavor::DILATE) {
            sstr << "    FragColor.rgb *= 2.0;" << std::endl;
        }
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

Lowpass::Lowpass(const Params2& params, LowpassFlavor flavor)
    : params_{ params }
    , flavor_{ flavor }
{}

Lowpass::~Lowpass() = default;

void Lowpass::render(
    int width,
    int height,
    const FixedArray<uint32_t, 2>& niterations,
    std::shared_ptr<FrameBuffer> fbs[2],
    size_t& target_id)
{
    for (auto&& [axis, rp] : enumerate(rp_.flat_iterable())) {
        if (!rp.allocated()) {
            rp.allocate(simple_vertex_shader_text_, filter_fragment_shader_text(params_(axis), flavor_, axis));
            rp.texture_color_location = rp.get_uniform_location("texture_color");
            rp.lowpass_offset_location = rp.get_uniform_location("offset");
        }
    }
    notify_rendering(CURRENT_SOURCE_LOCATION);
    for (auto&& [axis, rp] : enumerate(rp_.flat_iterable())) {
        rp.use();
        float offset;
        if (axis == 0) {
            offset = 1.f / integral_to_float<float>(width);
        } else if (axis == 1) {
            offset = 1.f / integral_to_float<float>(height);
        } else {
            THROW_OR_ABORT("Texture axis");
        }
        float offset2 = offset;
        for (size_t i = 0; i < niterations(axis); ++i) {
            size_t source_id = target_id;
            target_id = 1 - source_id;
            RenderToFrameBufferGuard rfg{ fbs[target_id] };
            ViewportGuard vg{ width, height };
            notify_rendering(CURRENT_SOURCE_LOCATION);

            TextureBinder tb;
            tb.bind(rp.texture_color_location, *fbs[source_id]->texture_color(), InterpolationPolicy::NEAREST_NEIGHBOR, TextureLayerProperties::NONE);
            CHK(glUniform1f(rp.lowpass_offset_location, offset2));
            offset2 *= 2.f;

            va().bind();
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));
        }
    }
}
