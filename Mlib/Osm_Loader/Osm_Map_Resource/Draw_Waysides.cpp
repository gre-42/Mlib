#include "Draw_Waysides.hpp"
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Contour.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Waysides_Surface.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const double dx = 0.01;

void Mlib::draw_waysides(
    BatchResourceInstantiator& bri,
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& inner_triangles,
    const GroundBvh& ground_bvh,
    const StreetBvh& entrance_bvh,
    double scale,
    const WaysideResourceNamesSurface& distances,
    ContourDetectionStrategy contour_detection_strategy)
{
    ResourceNameCycle rnc{distances.resource_names};
    FastNormalRandomNumberGenerator<float> scale_rng{ 0, 1.f, 0.2f };
    auto contours = find_contours(inner_triangles, contour_detection_strategy);

    for (const auto& contour_coarse : contours) {
        auto interp = interpolated_contour(contour_coarse);
        for (auto t = 2 * scale * dx; t <= interp.xmax() - 2 * scale * dx; t += scale * distances.tangential_distance) {
            auto d3 = funpack(interp(t + scale * dx) - interp(t - scale * dx));
            auto p3 = funpack(interp(t));

            FixedArray<double, 2> p2{ p3(0), p3(1) };
            if (entrance_bvh.has_neighbor(p2.casted<CompressedScenePos>(), (CompressedScenePos)1e-3)) {
                continue;
            }
            FixedArray<double, 2> n2{ d3(1), -d3(0) };
            auto n2_len = std::sqrt(sum(squared(n2)));
            if (n2_len < 1e-12) {
                THROW_OR_ABORT("draw_road_bollards: normal too short");
            }
            n2 /= n2_len;
            auto p2_shifted = (p2 + distances.normal_distance * n2 * scale).casted<CompressedScenePos>();
            auto yangle = (float)std::atan2(d3(1), d3(0));

            CompressedScenePos height;
            if (ground_bvh.height(height, p2_shifted)) {
                auto add_prn = [&](){
                    if (auto prn = rnc.try_multiple_times(10); prn != nullptr) {
                        bri.add_parsed_resource_name(p2_shifted, height, *prn, yangle, scale_rng());
                    }
                };
                if (std::isnan(distances.gradient_dx)) {
                    add_prn();
                } else {
                    FixedArray<double, 2> grad = uninitialized;
                    if (ground_bvh.gradient(grad, p2_shifted, (CompressedScenePos)(distances.gradient_dx * scale))) {
                        if (dot0d(grad, n2) <= distances.max_gradient) {
                            add_prn();
                        }
                    }
                }
            }
        }
    }
}
