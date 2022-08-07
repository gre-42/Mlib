#include "Supply_Depots.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

SupplyDepots::SupplyDepots(
    AdvanceTimes& advance_times,
    Players& players,
    const PhysicsEngineConfig& cfg)
: bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels},
  advance_times_{advance_times},
  players_{players},
  cfg_{cfg}
{}

SupplyDepots::~SupplyDepots()
{}

void SupplyDepots::handle_supply_depots() {
    for (auto& [_, player] : players_.players()) {
        if (!player->has_rigid_body()) {
            continue;
        }
        auto& rb = player->rigid_body();
        BoundingSphere<double, 3> bs(rb.rbi_.abs_position(), cfg_.supply_depot_attraction_radius);
        bvh_.visit(
            AxisAlignedBoundingBox{bs.center(), bs.radius()},
            [&](const SupplyDebot& supply_depot)
            {
                if (!bs.contains(supply_depot.center)) {
                    return true;
                }
                for (const auto& [item_type, navail] : supply_depot.supplies) {
                    uint32_t free = rb.inventory_.nfree(item_type);
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
    auto rt = std::make_shared<RelativeTransformer>(
        advance_times_,
        fixed_zeros<float, 3>(),
        FixedArray<float, 3>{0.f, 2.f * rpm, 0.f});
    scene_node.set_relative_movable(rt.get());
    advance_times_.add_advance_time(rt);
}
