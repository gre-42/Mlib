#include "Sample_Triangle_Interior_Instances.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

TriangleInteriorInstancesSampler::TriangleInteriorInstancesSampler(
    const TerrainStyle& terrain_style,
    double scale,
    const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh)
: tsc_{terrain_style.config},
  distances_to_bdry_{terrain_style.distances_to_bdry()},
  rnc_valley_{tsc_.near_resource_names_valley},
  rnc_mountain_{tsc_.near_resource_names_mountain},
  max_dboundary_{distances_to_bdry_.max_distance_to_bdry * scale},
  min_dboundary2_{squared(distances_to_bdry_.min_distance_to_bdry * scale)},
  ts_{ 392743 },
  scale_{scale},
  boundary_bvh_{boundary_bvh}
{
    assert_true(!tsc_.near_resource_names_valley.empty() ||
                !tsc_.near_resource_names_mountain.empty());
    assert_true(tsc_.much_near_distance != INFINITY);
}

void TriangleInteriorInstancesSampler::sample_triangle(
    const FixedArray<ColoredVertex<double>, 3>& t,
    unsigned int seed,
    const std::function<void(
        const FixedArray<double, 3>& p,
        const ParsedResourceName& prn)>& f)
{
    ts_.seed(392743 + seed);
    rnc_valley_.seed(4624052 + seed);
    rnc_mountain_.seed(283940 + seed);
    ts_.sample_triangle_interior(
        t(0).position,
        t(1).position,
        t(2).position,
        tsc_.much_near_distance * scale_,
        [&](const double& a, const double& b, const double& c)
        {
            FixedArray<float, 3> n = t(0).normal * float(a) + t(1).normal * float(b) + t(2).normal * float(c);
            bool is_in_valley = (squared(n(2)) > squared(0.85) * sum(squared(n)));
            if (is_in_valley && tsc_.near_resource_names_valley.empty()) {
                return;
            }
            if (!is_in_valley && tsc_.near_resource_names_mountain.empty()) {
                return;
            }
            FixedArray<double, 3> p = t(0).position * a + t(1).position * b + t(2).position * c;
            float min_dist2;
            if (distances_to_bdry_.is_active && (boundary_bvh_ != nullptr)) {
                min_dist2 = boundary_bvh_->min_distance(
                    p,
                    max_dboundary_,
                    [&p](auto& tt)
                    {
                        return sum(squared(distance_point_to_triangle_3d(
                            p,
                            tt(0),
                            tt(1),
                            tt(2))));
                    });
                if (min_dist2 < min_dboundary2_) {
                    return;
                }
            } else {
                min_dist2 = NAN;
            }
            auto& rnc = is_in_valley ? rnc_valley_ : rnc_mountain_;
            const ParsedResourceName* prn = rnc.try_multiple_times(
                10,  // nattempts
                LocationInformation{
                    .distance_to_boundary = std::isnan(min_dist2) ? NAN : std::sqrt(min_dist2) / (float)scale_});
            if (prn == nullptr) {
                return;
            }
            TransformationMatrix<float, double, 3> mi_rel{ fixed_identity_array<float, 3>(), p };
            f(p, *prn);
        });
}
