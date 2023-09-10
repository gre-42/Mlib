#include "Modulo_Uv.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Texture/Uv_Shifter.hpp>
#include <Mlib/Math/Least_Common_Multiple.hpp>
#include <list>

using namespace Mlib;

template <class TPos>
void Mlib::modulo_uv(ColoredVertexArray<TPos>& cva) {
    if (cva.material.textures.empty()) {
        return;
    }
    std::vector<float> lcm_world_args;
    FixedArray<std::vector<float>, 2> lcm_local_args;
    FixedArray<bool, 2> detected_non_repeat{false};
    lcm_world_args.reserve(cva.material.textures.size());
    lcm_local_args(0).reserve(cva.material.textures.size());
    lcm_local_args(1).reserve(cva.material.textures.size());
    for (const auto& t : cva.material.textures) {
        if (t.role == BlendMapRole::DETAIL_COLOR_HORIZONTAL) {
            lcm_world_args.push_back(1.f / t.scale);
        } else {
            for (size_t i = 0; i < 2; ++i) {
                if (t.texture_descriptor.wrap_modes(i) == WrapMode::REPEAT) {
                    lcm_local_args(i).push_back(1.f / t.scale);
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

namespace Mlib {
    template void modulo_uv<float>(ColoredVertexArray<float>& cva);
    template void modulo_uv<double>(ColoredVertexArray<double>& cva);
}
