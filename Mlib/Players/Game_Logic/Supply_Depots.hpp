#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Interfaces/ISupply_Depots.hpp>
#include <cstdint>
#include <memory>

namespace Mlib {

class Players;
class AdvanceTimes;
struct PhysicsEngineConfig;
class SceneNode;
class DestructionFunctionsRemovalTokens;

struct SupplyDepot {
    DanglingBaseClassRef<SceneNode> node;
    FixedArray<CompressedScenePos, 3> center;
    std::unordered_map<InventoryItem, uint32_t> initial_supplies;
    std::unordered_map<InventoryItem, uint32_t> remaining_supplies;
    float cooldown;
    float time_since_first_sale;
    std::shared_ptr<DestructionFunctionsRemovalTokens> node_on_clear;
    bool is_cooling_down() const;
    void notify_first_sale();
    void notify_reset();
};

class SupplyDepots: public ISupplyDepots {
public:
    SupplyDepots(
        AdvanceTimes& advance_times,
        Players& players,
        const PhysicsEngineConfig& cfg);
    ~SupplyDepots();
    void reset_cooldown();
    void handle_supply_depots(float dt);
    bool visit_supply_depots(
        const FixedArray<CompressedScenePos, 3> position,
        const std::function<bool(const SupplyDepot&)>& visitor) const;
    bool visit_supply_depots(
        const FixedArray<CompressedScenePos, 3> position,
        const std::function<bool(SupplyDepot&)>& visitor);
    virtual void add_supply_depot(
        const DanglingBaseClassRef<SceneNode>& scene_node,
        const std::unordered_map<InventoryItem, uint32_t>& supplies,
        float cooldown) override;
private:
    Bvh<CompressedScenePos, 3, SupplyDepot> bvh_;
    AdvanceTimes& advance_times_;
    Players& players_;
    const PhysicsEngineConfig& cfg_;
};

}
