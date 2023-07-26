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
    std::vector<float> lcm_args;
    lcm_args.reserve(cva.material.textures.size());
    for (const auto& t : cva.material.textures) {
        lcm_args.push_back(1.f / t.scale);
    }
    // linfo() << "period " << cva.material.identifier() << ": " << least_common_multiple(lcm_args.begin(), lcm_args.end(), 1e-6f, 1000);
    for (auto& tri : cva.triangles) {
        shift_uv3(
            least_common_multiple(lcm_args.begin(), lcm_args.end(), 1e-6f, 1000),
            tri(0).uv,
            tri(1).uv,
            tri(2).uv,
            {cva.material.wrap_mode_s, cva.material.wrap_mode_t});
    }
}

namespace Mlib {
    template void modulo_uv<float>(ColoredVertexArray<float>& cva);
    template void modulo_uv<double>(ColoredVertexArray<double>& cva);
}
