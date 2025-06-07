#include "Modulo_Uv.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Texture/Uv_Shifter.hpp>
#include <Mlib/Math/Least_Common_Multiple.hpp>
#include <list>

using namespace Mlib;

template <class TPos>
void Mlib::modulo_uv(ColoredVertexArray<TPos>& cva) {
    if (cva.material.textures_color.empty() && cva.material.textures_alpha.empty()) {
        return;
    }
    std::vector<const BlendMapTexture*> textures;
    textures.reserve(cva.material.textures_color.size() + cva.material.textures_alpha.size());
    for (const auto& t : cva.material.textures_color) {
        textures.push_back(&t);
    }
    for (const auto& t : cva.material.textures_alpha) {
        textures.push_back(&t);
    }
    std::vector<float> lcm_world_args;
    FixedArray<std::vector<float>, 2> lcm_local_args(std::vector<float>{});
    FixedArray<bool, 2> detected_non_repeat{false};
    lcm_world_args.reserve(textures.size());
    lcm_local_args(0).reserve(textures.size());
    lcm_local_args(1).reserve(textures.size());
    for (const auto& t : textures) {
        if (any(t->uv_source & BlendMapUvSource::ANY_HORIZONTAL)) {
            if (t->scale(0) != t->scale(1)) {
                THROW_OR_ABORT("Horizontal UV-coordinates require isotropic scaling. Material: " + cva.material.identifier());
            }
            float scale = t->scale(0);
            if (scale != 0.f) {
                lcm_world_args.push_back(1.f / scale);
            }
        } else {
            for (size_t i = 0; i < 2; ++i) {
                if (t->texture_descriptor.color.wrap_modes(i) == WrapMode::REPEAT) {
                    if (t->scale(i) != 0.f) {
                        lcm_local_args(i).push_back(1.f / t->scale(i));
                    }
                } else {
                    detected_non_repeat(i) = true;
                }
            }
        }
    }
    if (!lcm_world_args.empty()) {
        cva.material.period_world = least_common_multiple(lcm_world_args.begin(), lcm_world_args.end(), 1e-6f, 10'000);
        // linfo() << "period_world " << cva.material.identifier() << ": " << cva.material.period_world;
    }
    for (size_t i = 0; i < 2; ++i) {
        if (!lcm_local_args(i).empty()) {
            if (detected_non_repeat(i)) {
                THROW_OR_ABORT("Detected mixture of repeat/non-repeat wrap modes in material: " + cva.material.identifier());
            }
            auto period_local = least_common_multiple(lcm_local_args(i).begin(), lcm_local_args(i).end(), 1e-6f, 10'000);
            // linfo() << "period_local " << cva.material.identifier() << ": " << period_local;
            for (auto& tri : cva.triangles) {
                shift_uv3(
                    period_local,
                    tri(0).uv,
                    tri(1).uv,
                    tri(2).uv,
                    WrapMode::CLAMP_TO_EDGE,
                    i);
            }
        }
    }
}

template void Mlib::modulo_uv<float>(ColoredVertexArray<float>& cva);
template void Mlib::modulo_uv<CompressedScenePos>(ColoredVertexArray<CompressedScenePos>& cva);
