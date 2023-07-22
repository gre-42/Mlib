#include "Modulo_Uv.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Texture/Uv_Shifter.hpp>
#include <list>

using namespace Mlib;

template <class TPos>
void Mlib::modulo_uv(ColoredVertexArray<TPos>& cva) {
    for (auto& tri : cva.triangles) {
        shift_uv3(
            1.f,
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
