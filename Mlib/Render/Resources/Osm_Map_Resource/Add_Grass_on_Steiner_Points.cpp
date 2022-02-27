#include "Add_Grass_on_Steiner_Points.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Resource_Name_Cycle.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Scene_Graph/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_grass_on_steiner_points(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const StreetBvh& ground_bvh,
    const StreetBvh& air_bvh,
    const std::list<SteinerPointInfo>& steiner_points,
    float scale,
    float dmin,
    float dmax)
{
    NormalRandomNumberGenerator<float> scale_rng{0, 1.f, 0.2f};
    for (const auto& p : steiner_points) {
        if (p.type == SteinerPointType::STREET_NEIGHBOR) {
            FixedArray<float, 2> pt{ p.position(0), p.position(1) };
            float distance_to_road = ground_bvh.min_dist(pt, dmax * scale);
            float distance_to_air_road = air_bvh.min_dist(pt, dmin * scale);
            if (!std::isnan(distance_to_road) &&
                ((distance_to_road > dmin * scale) &&
                (distance_to_road < dmax * scale) &&
                (distance_to_air_road > dmin * scale)))
            {
                const ParsedResourceName* prn = rnc.try_once();
                if (prn != nullptr) {
                    bri.add_parsed_resource_name(p.position, *prn, 0.f, scale_rng());
                }
            }
        }
    }
}
