
#include "Lowpass.hpp"
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
        sstr << "    vec4 center = texture(texture_color, TexCoords.st).rgba;" << std::endl;
        if (flavor == LowpassFlavor::EXTRAPOLATE) {
            sstr << "    if (center.a > 1e-2) {" << std::endl;
            sstr << "        FragColor = vec4(center.rgb / center.a, 1.0);" << std::endl;
            sstr << "    } else {" << std::endl;
        } else {
            sstr << "    {" << std::endl;
        }
        std::visit(overloads{
            [&](const BoxParameters&){
                if (axis == 0) {
                    sstr << "        vec4 left  = texture(texture_color, TexCoords.st - vec2(offset, 0.0)).rgba;" << std::endl;
                    sstr << "        vec4 right = texture(texture_color, TexCoords.st + vec2(offset, 0.0)).rgba;" << std::endl;
                } else if (axis == 1) {
                    sstr << "        vec4 left  = texture(texture_color, TexCoords.st - vec2(0.0, offset)).rgba;" << std::endl;
                    sstr << "        vec4 right = texture(texture_color, TexCoords.st + vec2(0.0, offset)).rgba;" << std::endl;
                } else {
                    throw std::runtime_error("Unknown texture axis");
                }
                if (flavor == LowpassFlavor::EXTRAPOLATE) {
                    sstr << "        FragColor.a = center.a + left.a + right.a;" << std::endl;
                    sstr << "        FragColor.rgb =" << std::endl;
                    sstr << "            center.rgb * center.a +" << std::endl;
                    sstr << "            left.rgb * left.a +" << std::endl;
                    sstr << "            right.rgb * right.a;" << std::endl;
                } else {
                    sstr << "        FragColor = vec4((center.rgb + left.rgb + right.rgb) / 3.0, 1.0);" << std::endl;
                }
            },
            [&](const NormalParameters& params){
                auto weights = gaussian_kernel(params.stddev, 3.5f);
                int cdist = integral_cast<int>(weights.length() / 2);
                if (flavor == LowpassFlavor::EXTRAPOLATE) {
                    sstr << "        FragColor = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
                } else {
                    sstr << "        FragColor = vec4(0.0, 0.0, 0.0, 1.0);" << std::endl;
                }
                for (const auto& [i, w] : tenumerate<int>(weights.flat_iterable())) {
                    sstr << "        {" << std::endl;
                    if (axis == 0) {
                        sstr <<
                            "            vec4 tex =" <<
                            " texture(texture_color, TexCoords.st + vec2(" <<
                            integral_to_float<float>(i - cdist) << " * offset, 0.0)).rgba;" << std::endl;
                    } else if (axis == 1) {
                        sstr <<
                            "            vec4 tex ="
                            " texture(texture_color, TexCoords.st + vec2(0.0, " <<
                            integral_to_float<float>(i - cdist) << " * offset)).rgba;" << std::endl;
                    } else {
                        throw std::runtime_error("Unknown texture axis");
                    }
                    if (flavor == LowpassFlavor::EXTRAPOLATE) {
                        sstr << "            float wa = " << w << " * tex.a;" << std::endl;
                        sstr << "            FragColor.a += wa;" << std::endl;
                        sstr << "            FragColor.rgb += wa * tex.rgb;" << std::endl;
                    } else {
                        sstr << "            FragColor.rgb += " << w << " * tex.rgb;" << std::endl;
                    }
                    sstr << "        }" << std::endl;
                }
            },
            }, params);
        [&](){
            switch (flavor) {
            case LowpassFlavor::NONE:
                return;
            case LowpassFlavor::MAX:
                sstr << "        FragColor.rgb = max(FragColor.rgb, center);" << std::endl;
                return;
            case LowpassFlavor::DILATE:
                sstr << "        FragColor.rgb *= 2.0;" << std::endl;
                return;
            case LowpassFlavor::EXTRAPOLATE:
                sstr << "        if (FragColor.a > 1e-2) {" << std::endl;
                sstr << "            FragColor = vec4(FragColor.rgb / FragColor.a, 1.0);" << std::endl;
                sstr << "        } else {" << std::endl;
                sstr << "            FragColor = center;" << std::endl;
                sstr << "        }" << std::endl;
                return;
            }
            throw std::runtime_error("Unknown lowpass flavor");
        }();
        sstr << "    }" << std::endl;
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
            throw std::runtime_error("Texture axis");
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

FixedArray<uint32_t, 2> Lowpass::full_extrapolation_niterations(
    int width,
    int height) const
{
    return {
        float_to_integral<uint32_t>(std::ceil(std::log2(width))),
        float_to_integral<uint32_t>(std::ceil(std::log2(height)))};
}
