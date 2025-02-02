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
    ScenePos scale,
    UpAxis up_axis,
    const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>* boundary_bvh,
    const Array<float>& dirtmap,
    float dirtmap_scale,
    const Array<float>& mudmap)
    : tsc_{terrain_style.config}
    , distances_to_bdry_{terrain_style.distances_to_bdry()}
    , rnc_valley_regular_{tsc_.near_resource_names_valley_regular}
    , rnc_mountain_regular_{tsc_.near_resource_names_mountain_regular}
    , rnc_valley_dirt_{tsc_.near_resource_names_valley_dirt}
    , rnc_mountain_dirt_{tsc_.near_resource_names_mountain_dirt}
    , max_dboundary_{distances_to_bdry_.max_distance_to_bdry * scale}
    , min_dboundary2_{squared(distances_to_bdry_.min_distance_to_bdry * scale)}
    , ts_{ 392743 }
    , scale_{scale}
    , up_axis_{up_axis}
    , boundary_bvh_{boundary_bvh}
    , dirtmap_{dirtmap}
    , dirtmap_scale_{dirtmap_scale}
    , mudmap_{mudmap}
{
    assert_true(!tsc_.near_resource_names_valley_regular.empty() ||
                !tsc_.near_resource_names_mountain_regular.empty() ||
                !tsc_.near_resource_names_valley_dirt.empty() ||
                !tsc_.near_resource_names_mountain_dirt.empty());
    assert_true(tsc_.much_near_distance != INFINITY);
}

void TriangleInteriorInstancesSampler::sample_triangle(
    const FixedArray<ColoredVertex<CompressedScenePos>, 3>& t,
    unsigned int seed,
    const std::function<void(
        const FixedArray<CompressedScenePos, 3>& p,
        const ParsedResourceName& prn)>& f)
{
    ts_.seed(392743 + seed);
    rnc_valley_regular_.seed(4624052 + seed);
    rnc_mountain_regular_.seed(283940 + seed);
    rnc_valley_dirt_.seed(76218 + seed);
    rnc_mountain_dirt_.seed(3846 + seed);
    auto triangle = funpack(FixedArray<CompressedScenePos, 3, 3>{
        t(0).position,
        t(1).position,
        t(2).position});
    auto uvs = FixedArray<float, 3, 2>{
        t(0).uv,
        t(1).uv,
        t(2).uv};
    auto normals = FixedArray<float, 3, 3>{
        t(0).normal,
        t(1).normal,
        t(2).normal};
    ts_.sample_triangle_interior(
        triangle,
        tsc_.much_near_distance * scale_,
        [&](const FixedArray<ScenePos, 3>& bc)
        {
            if (mudmap_.initialized()) {
                if ((mudmap_.shape(0) == 0) ||
                    (mudmap_.shape(1) == 0))
                {
                    THROW_OR_ABORT("Mudmap dimension is zero");
                }
                FixedArray<float, 2> uv = dot(bc.casted<float>(), uvs);
                uv(0) -= std::floor(uv(0));
                uv(1) -= std::floor(uv(1));
                uv(1) = 1 - uv(1);
                float intensity;
                if (!bilinear_grayscale_interpolation(uv(1) * float(mudmap_.shape(0) - 1), uv(0) * float(mudmap_.shape(1) - 1), mudmap_, intensity)) {
                    THROW_OR_ABORT("Unexpected bilinear interpolation failure (0)");
                }
                if (intensity >= 0.5f) {
                    return;
                }
            }
            FixedArray<float, 3> n = dot(bc.casted<float>(), normals);
            bool is_in_valley = (squared(n((size_t)up_axis_)) > squared(0.85) * sum(squared(n)));
            bool is_regular;
            if (dirtmap_.initialized()) {
                if ((dirtmap_.shape(0) == 0) ||
                    (dirtmap_.shape(1) == 0))
                {
                    THROW_OR_ABORT("Dirtmap dimension is zero");
                }
                FixedArray<float, 2> uv = dot(bc.casted<float>(), uvs);
                uv *= dirtmap_scale_;
                uv(0) -= std::floor(uv(0));
                uv(1) -= std::floor(uv(1));
                uv(1) = 1 - uv(1);
                float intensity;
                if (!bilinear_grayscale_interpolation(uv(1) * float(dirtmap_.shape(0) - 1), uv(0) * float(dirtmap_.shape(1) - 1), dirtmap_, intensity)) {
                    THROW_OR_ABORT("Unexpected bilinear interpolation failure (1)");
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
            FixedArray<ScenePos, 3> up = dot(bc, triangle);
            FixedArray<CompressedScenePos, 3> p = up.casted<CompressedScenePos>();
            float min_dist2;
            if (distances_to_bdry_.is_active && (boundary_bvh_ != nullptr)) {
                auto md2 = boundary_bvh_->min_distance<FixedArray<CompressedScenePos, 3, 3>>(
                    p,
                    max_dboundary_,
                    [&up](auto& tt)
                    {
                        return sum(squared(distance_point_to_triangle_3d(up, funpack(tt))));
                    });
                min_dist2 = md2.has_value()
                    ? (float)(*md2)
                    : INFINITY;
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
