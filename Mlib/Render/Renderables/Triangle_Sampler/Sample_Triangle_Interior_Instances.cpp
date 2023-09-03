#include "Sample_Triangle_Interior_Instances.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Up_Axis.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

TriangleInteriorInstancesSampler::TriangleInteriorInstancesSampler(
    const TerrainStyle& terrain_style,
    double scale,
    UpAxis up_axis,
    const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh,
    const Array<float>& dirtmap,
    float dirtmap_scale)
: tsc_{terrain_style.config},
  distances_to_bdry_{terrain_style.distances_to_bdry()},
  rnc_valley_regular_{tsc_.near_resource_names_valley_regular},
  rnc_mountain_regular_{tsc_.near_resource_names_mountain_regular},
  rnc_valley_dirt_{tsc_.near_resource_names_valley_dirt},
  rnc_mountain_dirt_{tsc_.near_resource_names_mountain_dirt},
  max_dboundary_{distances_to_bdry_.max_distance_to_bdry * scale},
  min_dboundary2_{squared(distances_to_bdry_.min_distance_to_bdry * scale)},
  ts_{ 392743 },
  scale_{scale},
  up_axis_{up_axis},
  boundary_bvh_{boundary_bvh},
  dirtmap_{dirtmap},
  dirtmap_scale_{dirtmap_scale}
{
    assert_true(!tsc_.near_resource_names_valley_regular.empty() ||
                !tsc_.near_resource_names_mountain_regular.empty() ||
                !tsc_.near_resource_names_valley_dirt.empty() ||
                !tsc_.near_resource_names_mountain_dirt.empty());
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
    rnc_valley_regular_.seed(4624052 + seed);
    rnc_mountain_regular_.seed(283940 + seed);
    rnc_valley_dirt_.seed(76218 + seed);
    rnc_mountain_dirt_.seed(3846 + seed);
    ts_.sample_triangle_interior(
        t(0).position,
        t(1).position,
        t(2).position,
        tsc_.much_near_distance * scale_,
        [&](const double& a, const double& b, const double& c)
        {
            FixedArray<float, 3> n = t(0).normal * float(a) + t(1).normal * float(b) + t(2).normal * float(c);
            bool is_in_valley = (squared(n((size_t)up_axis_)) > squared(0.85) * sum(squared(n)));
            bool is_regular;
            if (dirtmap_.initialized()) {
                if ((dirtmap_.shape(0) == 0) ||
                    (dirtmap_.shape(1) == 0))
                {
                    THROW_OR_ABORT("Dirtmap dimension is zero");
                }
                FixedArray<float, 2> uv = t(0).uv * float(a) + t(1).uv * float(b) + t(2).uv * float(c);
                uv *= dirtmap_scale_;
                uv(0) -= std::floor(uv(0));
                uv(1) -= std::floor(uv(1));
                float intensity;
                if (!bilinear_grayscale_interpolation(uv(1) * float(dirtmap_.shape(0) - 1), uv(0) * float(dirtmap_.shape(1) - 1), dirtmap_, intensity)) {
                    THROW_OR_ABORT("Unexpected bilinear interpolation failure");
                }
                is_regular = (intensity < 0.5f);
            } else {
                is_regular = true;
            }
            if (is_in_valley && is_regular && tsc_.near_resource_names_valley_regular.empty()) {
                return;
            }
            if (!is_in_valley && is_regular && tsc_.near_resource_names_mountain_regular.empty()) {
                return;
            }
            if (is_in_valley && !is_regular && tsc_.near_resource_names_valley_dirt.empty()) {
                return;
            }
            if (!is_in_valley && !is_regular && tsc_.near_resource_names_mountain_dirt.empty()) {
                return;
            }
            FixedArray<double, 3> p = t(0).position * a + t(1).position * b + t(2).position * c;
            float min_dist2;
            if (distances_to_bdry_.is_active && (boundary_bvh_ != nullptr)) {
                min_dist2 = (float)boundary_bvh_->min_distance(
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
            auto& rnc = is_in_valley
                ? (is_regular ? rnc_valley_regular_ : rnc_valley_dirt_)
                : (is_regular ? rnc_mountain_regular_ : rnc_mountain_dirt_);
            const ParsedResourceName* prn = rnc.try_multiple_times(
                10,  // nattempts
                LocationInformation{
                    .distance_to_boundary = std::isnan(min_dist2) ? NAN : std::sqrt(min_dist2) / (float)scale_});
            if (prn == nullptr) {
                return;
            }
            f(p, *prn);
        });
}
