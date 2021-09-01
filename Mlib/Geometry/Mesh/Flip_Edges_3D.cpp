#include "Flip_Edges_3D.hpp"
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <map>

using namespace Mlib;

typedef OrderableFixedArray<OrderableFixedArray<float, 3>, 2> Edge3;
typedef FixedArray<FixedArray<float, 3>, 3> Triangle3;

Array<FixedArray<FixedArray<float, 3>, 3>> Mlib::flip_edges_3d(
    const Array<FixedArray<FixedArray<float, 3>, 3>>& mesh)
{
    std::map<Edge3, Triangle3> edges;
    for (const auto& t : mesh.flat_iterable()) {
        for (size_t i = 0; i < 3; ++i) {
            Edge3 edge{
                OrderableFixedArray{t(i)},
                OrderableFixedArray{t((i + 1) % 3)}};
            if (!edges.insert({edge, t}).second) {
                std::cerr << "Flip edges 3D: Detected duplicate edge" << std::endl;
            }
        }
    }
    Array<FixedArray<FixedArray<float, 3>, 3>> result{ ArrayShape{ 0 } };
    for (const auto& e : edges) {
        auto it = edges.find({Edge3{e.first(1), e.first(0)}});
        if (it == edges.end()) {
            result.append(e.second);
        } else if (e.first(0) < e.first(1)) {
            float cc_max = std::max(
                triangle_largest_cosine(e.second),
                triangle_largest_cosine(it->second));

            size_t pi0;
            for (pi0 = 0; pi0 < 2; ++pi0) {
                if (all(e.second(pi0) == e.first(0))) {
                    break;
                }
            }
            pi0 = (pi0 + 2) % 3;
            size_t pi1;
            for (pi1 = 0; pi1 < 2; ++pi1) {
                if (all(it->second(pi1) == e.first(1))) {
                    break;
                }
            }
            pi1 = (pi1 + 2) % 3;
            FixedArray<FixedArray<float, 3>, 3> tri0_flipped{
                e.second(pi0),
                e.second((pi0 + 1) % 3),
                it->second(pi1)};
            FixedArray<FixedArray<float, 3>, 3> tri1_flipped{
                it->second(pi1),
                it->second((pi1 + 1) % 3),
                e.second(pi0)};
            float ca_max = std::max(
                triangle_largest_cosine(tri0_flipped),
                triangle_largest_cosine(tri1_flipped));
            if (cc_max < ca_max) {
                result.append(e.second);
                result.append(it->second);
            } else {
                result.append(tri0_flipped);
                result.append(tri1_flipped);
            }
        }
    }
    return result;
}
