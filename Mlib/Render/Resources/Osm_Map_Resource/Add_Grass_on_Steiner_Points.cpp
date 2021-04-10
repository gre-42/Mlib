#include "Add_Grass_on_Steiner_Points.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_grass_on_steiner_points(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    ResourceNameCycle& rnc,
    const std::list<SteinerPointInfo>& steiner_points,
    float scale,
    float dmin,
    float dmax)
{
    NormalRandomNumberGenerator<float> scale_rng{0, 1.f, 0.2f};
    for (const auto& p : steiner_points) {
        if ((p.type == SteinerPointType::STREET_NEIGHBOR) &&
            !std::isnan(p.distance_to_road) &&
            ((p.distance_to_road > dmin * scale) &&
             (p.distance_to_road < dmax * scale) &&
             (p.distance_to_air_road > dmin * scale)))
        {
            const ParsedResourceName* prn = rnc.try_once();
            if (prn != nullptr) {
                add_parsed_resource_name(p.position, *prn, 0.f, scale_rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
            }
        }
    }
}
