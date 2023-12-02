#include "Draw_Road_Bollards.hpp"
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Contour.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void Mlib::draw_road_bollards(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const std::list<FixedArray<ColoredVertex<double>, 3>>& inner_triangles,
    const GroundBvh& ground_bvh,
    const StreetBvh& entrance_bvh,
    double scale,
    double tangential_distance,
    double normal_distance,
    double gradient_dx,
    double max_gradient)
{
    FastNormalRandomNumberGenerator<float> scale_rng{ 0, 1.f, 0.2f };
    auto contours = find_contours(inner_triangles, ContourDetectionStrategy::NODE_NEIGHBOR);

    for (const auto& contour_coarse : contours) {
        auto subdiv_contour = subdivided_contour(contour_coarse, scale, tangential_distance);
        if (subdiv_contour.size() < 4) {
            continue;
        }
        auto inc_it = [&subdiv_contour](auto& it){
            if (++it == subdiv_contour.end()) {
                it = subdiv_contour.begin();
            }
        };
        auto it2 = subdiv_contour.begin();
        auto it0 = it2++;
        auto it1 = it2++;
        while (it1 != subdiv_contour.begin()) {
            const auto d3 = *it2 - *it0;
            const auto& p3 = *it1;

            inc_it(it0);
            inc_it(it1);
            inc_it(it2);

            FixedArray<double, 2> p2{ p3(0), p3(1) };
            if (entrance_bvh.has_neighbor(p2, 1e-12)) {
                continue;
            }
            FixedArray<double, 2> n2{ d3(1), -d3(0) };
            auto n2_len = std::sqrt(sum(squared(n2)));
            if (n2_len < 1e-12) {
                THROW_OR_ABORT("draw_road_bollards: normal too short");
            }
            n2 /= n2_len;
            auto p2_shifted = p2 + normal_distance * n2 * scale;
            auto yangle = (float)std::atan2(d3(1), d3(0));

            double height;
            if (ground_bvh.height(height, p2_shifted)) {
                FixedArray<double, 2> grad;
                if (ground_bvh.gradient(grad, p2_shifted, gradient_dx * scale)) {
                    if (dot0d(grad, n2) <= max_gradient) {
                        if (auto prn = rnc.try_multiple_times(10); prn != nullptr) {
                            bri.add_parsed_resource_name(p2_shifted, height, *prn, yangle, scale_rng());
                        }
                    }
                }
            }
        }
    }
}
