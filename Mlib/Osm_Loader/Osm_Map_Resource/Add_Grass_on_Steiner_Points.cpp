#include "Add_Grass_on_Steiner_Points.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>

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
    FastNormalRandomNumberGenerator<float> scale_rng{0, 1.f, 0.2f};
    for (const auto& p : steiner_points) {
        if (p.type == SteinerPointType::STREET_NEIGHBOR) {
            FixedArray<CompressedScenePos, 2> pt{ p.position(0), p.position(1) };
            auto distance_to_road = ground_bvh.min_dist(pt, (CompressedScenePos)(dmax * scale)).value_or(std::numeric_limits<CompressedScenePos>::max());
            auto distance_to_air_road = air_bvh.min_dist(pt, (CompressedScenePos)(dmin * scale)).value_or(std::numeric_limits<CompressedScenePos>::max());
            if ((distance_to_road > (CompressedScenePos)(dmin * scale)) &&
                (distance_to_road < (CompressedScenePos)(dmax * scale)) &&
                (distance_to_air_road > (CompressedScenePos)(dmin * scale)))
            {
                const ParsedResourceName* prn = rnc.try_once();
                if (prn != nullptr) {
                    bri.add_parsed_resource_name(p.position, *prn, 0.f, scale_rng());
                }
            }
        }
    }
}
