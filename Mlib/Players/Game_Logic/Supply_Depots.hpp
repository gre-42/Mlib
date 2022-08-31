#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Graph/Interfaces/ISupplyDepots.hpp>
#include <cstdint>
#include <map>

namespace Mlib {

class Players;
class AdvanceTimes;
struct PhysicsEngineConfig;
class SceneNode;

struct SupplyDepot {
    SceneNode& node;
    FixedArray<double, 3> center;
    std::map<std::string, uint32_t> supplies;
    float cooldown;
    float time_since_last_visit;
    bool is_cooling_down() const;
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
        const FixedArray<double, 3> position,
        const std::function<bool(const SupplyDepot&)>& visitor);
    virtual void add_supply_depot(
        SceneNode& scene_node,
        const std::map<std::string, uint32_t>& supplies,
        float cooldown) override;
private:
    Bvh<double, SupplyDepot, 3> bvh_;
    AdvanceTimes& advance_times_;
    Players& players_;
    const PhysicsEngineConfig& cfg_;
};

}
