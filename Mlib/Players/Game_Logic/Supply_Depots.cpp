#include "Supply_Depots.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

SupplyDepots::SupplyDepots(Players& players, const PhysicsEngineConfig& cfg)
: bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, 7},
  players_{players}
{}

SupplyDepots::~SupplyDepots()
{}

void SupplyDepots::handle_supply_depots() {
    for (auto& [_, player] : players_.players()) {
        if (!player->has_rigid_body()) {
            continue;
        }
        auto& rb = player->rigid_body();
        BoundingSphere<double, 3> bs(rb.rbi_.abs_position(), 10.0 * meters);
        bvh_.visit(
            AxisAlignedBoundingBox{bs.center(), bs.radius()},
            [&](const SupplyDebot& supply_depot)
            {
                if (!bs.contains(supply_depot.center)) {
                    return true;
                }
                for (const auto& [item_type, navail] : supply_depot.supplies) {
                    uint32_t free = rb.inventory_.free_amount(item_type);
                    rb.inventory_.add(item_type, std::min(free, navail));
                }
                return true;
            });
    }
}

void SupplyDepots::add_supply_depot(
    SceneNode& scene_node,
    const std::map<std::string, uint32_t>& supplies)
{
    auto center = scene_node.absolute_model_matrix().t();
    bvh_.insert(
        AxisAlignedBoundingBox<double, 3>{center},
        SupplyDebot{.center = center, .supplies = supplies});
}
