#include "Supply_Depots.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

bool SupplyDepot::is_cooling_down() const {
    return time_since_last_visit < cooldown;
}

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

void SupplyDepots::reset_cooldown() {
    bvh_.visit_all([](const auto& aabb, SupplyDepot& supply_depot){
        supply_depot.time_since_last_visit = supply_depot.cooldown;
        supply_depot.node->color_style("").emissive = -1.f;
        return true;
    });
}

bool SupplyDepots::visit_supply_depots(
    const FixedArray<double, 3> position,
    const std::function<bool(const SupplyDepot&)>& visitor) const
{
    BoundingSphere<double, 3> bs(position, cfg_.supply_depot_attraction_radius);
    return bvh_.visit(
        AxisAlignedBoundingBox{bs.center(), bs.radius()},
        [&](const SupplyDepot& supply_depot)
        {
            if (supply_depot.is_cooling_down()) {
                return true;
            }
            if (!bs.contains(supply_depot.center)) {
                return true;
            }
            return visitor(supply_depot);
        });
}

bool SupplyDepots::visit_supply_depots(
    const FixedArray<double, 3> position,
    const std::function<bool(SupplyDepot&)>& visitor)
{
    const SupplyDepots& sd = *this;
    return sd.visit_supply_depots(
        position,
        [&visitor](const SupplyDepot& supply_depot){return visitor(const_cast<SupplyDepot&>(supply_depot));});
}

void SupplyDepots::handle_supply_depots(float dt) {
    bvh_.visit_all([&dt](const auto& aabb, SupplyDepot& supply_depot){
        bool old_cd = supply_depot.is_cooling_down();
        supply_depot.time_since_last_visit += dt;
        if (old_cd && !supply_depot.is_cooling_down()) {
            supply_depot.node->color_style("").emissive = -1.f;
        }
        return true;
    });
    for (auto& [_, player] : players_.players()) {
        if (!player->has_scene_vehicle()) {
            continue;
        }
        auto& rb = player->rigid_body();
        visit_supply_depots(
            rb.rbp_.abs_position(),
            [&rb](SupplyDepot& supply_depot)
            {
                for (const auto& [item_type, navail] : supply_depot.supplies) {
                    if (!rb.inventory_.knows_item_type(item_type)) {
                        continue;
                    }
                    uint32_t free = rb.inventory_.nfree(item_type);
                    rb.inventory_.add(item_type, std::min(free, navail));
                }
                supply_depot.time_since_last_visit = 0.f;
                supply_depot.node->color_style("").emissive = 2.f;
                return true;
            });
    }
}

void SupplyDepots::add_supply_depot(
    DanglingRef<SceneNode> scene_node,
    const std::map<std::string, uint32_t>& supplies,
    float cooldown)
{
    auto center = scene_node->absolute_model_matrix().t();
    auto payload = bvh_.insert(
        AxisAlignedBoundingBox<double, 3>{center},
        SupplyDepot{
            .node = scene_node,
            .center = center,
            .supplies = supplies,
            .cooldown = cooldown,
            .time_since_last_visit = cooldown,
            .node_on_clear = std::make_shared<DestructionFunctionsRemovalTokens>(scene_node->on_clear) });
    auto rt = std::make_unique<RelativeTransformer>(
        advance_times_,
        fixed_zeros<float, 3>(),
        FixedArray<float, 3>{0.f, 2.f * rpm, 0.f});
    scene_node->add_color_style(std::unique_ptr<ColorStyle>(new ColorStyle{.selector = Mlib::compile_regex("")}));
    scene_node->set_relative_movable({*rt, CURRENT_SOURCE_LOCATION});
    payload->node_on_clear->add([this](){ bvh_.clear(); });
    advance_times_.add_advance_time(std::move(rt));
}
