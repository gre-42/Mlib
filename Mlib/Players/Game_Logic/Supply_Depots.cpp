#include "Supply_Depots.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

bool SupplyDepot::is_cooling_down() const {
    return time_since_first_sale < cooldown;
}

void SupplyDepot::notify_first_sale() {
    time_since_first_sale = 0.f;
    node->color_style(VariableAndHash<std::string>{""}).set_emissive(fixed_full<float, 3>(2.f));
}

void SupplyDepot::notify_reset() {
    remaining_supplies = initial_supplies;
    time_since_first_sale = cooldown;
    node->color_style(VariableAndHash<std::string>{""}).set_emissive(fixed_full<float, 3>(-1.f));
}

SupplyDepots::SupplyDepots(
    AdvanceTimes& advance_times,
    Players& players,
    const PhysicsEngineConfig& cfg)
    : bvh_{{cfg.bvh_max_size,
            cfg.bvh_max_size,
            cfg.bvh_max_size},
        cfg.bvh_levels }
    , advance_times_{ advance_times }
    , players_{ players }
    , cfg_{ cfg }
{}

SupplyDepots::~SupplyDepots()
{}

void SupplyDepots::reset_cooldown() {
    bvh_.visit_all([](const auto& d){
        auto& payload = const_cast<SupplyDepot&>(d.payload());
        payload.notify_reset();
        return true;
    });
}

bool SupplyDepots::visit_supply_depots(
    const FixedArray<CompressedScenePos, 3> position,
    const std::function<bool(const SupplyDepot&)>& visitor) const
{
    BoundingSphere<CompressedScenePos, 3> bs(position, cfg_.supply_depot_attraction_radius);
    return bvh_.visit(
        AxisAlignedBoundingBox<CompressedScenePos, 3>::from_center_and_radius(bs.center, bs.radius),
        [&](const SupplyDepot& supply_depot)
        {
            // if (supply_depot.is_cooling_down()) {
            //     return true;
            // }
            if (!bs.contains(supply_depot.center)) {
                return true;
            }
            return visitor(supply_depot);
        });
}

bool SupplyDepots::visit_supply_depots(
    const FixedArray<CompressedScenePos, 3> position,
    const std::function<bool(SupplyDepot&)>& visitor)
{
    const SupplyDepots& sd = *this;
    return sd.visit_supply_depots(
        position,
        [&visitor](const SupplyDepot& supply_depot){ return visitor(const_cast<SupplyDepot&>(supply_depot)); });
}

void SupplyDepots::handle_supply_depots(float dt) {
    bvh_.visit_all([&dt](const auto& entry){
        auto& supply_depot = const_cast<SupplyDepot&>(entry.payload());
        bool old_cd = supply_depot.is_cooling_down();
        supply_depot.time_since_first_sale += dt;
        if (old_cd && !supply_depot.is_cooling_down()) {
            supply_depot.notify_reset();
        }
        return true;
    });
    for (auto& [_, player] : players_.players()) {
        if (!player->has_scene_vehicle()) {
            continue;
        }
        auto rb = player->rigid_body();
        visit_supply_depots(
            rb->rbp_.abs_position().casted<CompressedScenePos>(),
            [&rb](SupplyDepot& supply_depot)
            {
                bool cd = supply_depot.is_cooling_down();
                for (auto& [item_type, navail] : supply_depot.remaining_supplies) {
                    if (!rb->inventory_.knows_item_type(item_type)) {
                        continue;
                    }
                    uint32_t free = rb->inventory_.nfree(item_type);
                    uint32_t ntaken = std::min(free, navail);
                    if (ntaken != 0) {
                        rb->inventory_.add(item_type, ntaken);
                        navail -= ntaken;
                        if (!cd) {
                            supply_depot.notify_first_sale();
                            cd = true;
                        }
                    }
                }
                return true;
            });
    }
}

void SupplyDepots::add_supply_depot(
    const DanglingBaseClassRef<SceneNode>& scene_node,
    const std::unordered_map<InventoryItem, uint32_t>& supplies,
    float cooldown)
{
    auto center = scene_node->absolute_model_matrix().t.casted<CompressedScenePos>();
    auto& element = bvh_.insert(
        AxisAlignedBoundingBox<CompressedScenePos, 3>::from_point(center),
        SupplyDepot{
            .node = scene_node,
            .center = center,
            .initial_supplies = supplies,
            .remaining_supplies = supplies,
            .cooldown = cooldown,
            .time_since_first_sale = cooldown,
            .node_on_clear = std::make_shared<DestructionFunctionsRemovalTokens>(scene_node->on_clear, CURRENT_SOURCE_LOCATION) });
    auto& rt = global_object_pool.create<RelativeTransformer>(
        CURRENT_SOURCE_LOCATION,
        fixed_zeros<float, 3>(),
        FixedArray<float, 3>{0.f, 2.f * rpm, 0.f});
    scene_node->add_color_style(std::make_unique<ColorStyle>());
    scene_node->set_relative_movable({ rt, CURRENT_SOURCE_LOCATION });
    element.payload().node_on_clear->add([this](){ bvh_.clear(); }, CURRENT_SOURCE_LOCATION);
    advance_times_.add_advance_time({ rt, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}
