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
    std::vector<float> lcm_local_args;
    lcm_world_args.reserve(cva.material.textures.size());
    lcm_local_args.reserve(cva.material.textures.size());
    for (const auto& t : cva.material.textures) {
        if (t.role == BlendMapRole::DETAIL_COLOR_HORIZONTAL) {
            lcm_world_args.push_back(1.f / t.scale);
        } else {
            lcm_local_args.push_back(1.f / t.scale);
        }
    }
    if (!lcm_world_args.empty()) {
        cva.material.period_world = least_common_multiple(lcm_world_args.begin(), lcm_world_args.end(), 1e-6f, 1000);
        // linfo() << "period_world " << cva.material.identifier() << ": " << cva.material.period_world;
    }
    if (!lcm_local_args.empty()) {
        auto period_local = least_common_multiple(lcm_local_args.begin(), lcm_local_args.end(), 1e-6f, 1000);
        // linfo() << "period_local " << cva.material.identifier() << ": " << period_local;
        for (auto& tri : cva.triangles) {
            shift_uv3(
                period_local,
                tri(0).uv,
                tri(1).uv,
                tri(2).uv,
                {cva.material.wrap_mode_s, cva.material.wrap_mode_t});
        }
    }
}

namespace Mlib {
    template void modulo_uv<float>(ColoredVertexArray<float>& cva);
    template void modulo_uv<double>(ColoredVertexArray<double>& cva);
}
