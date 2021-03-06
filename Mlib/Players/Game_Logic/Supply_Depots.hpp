#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Graph/Interfaces/ISupplyDepots.hpp>
#include <cstdint>
#include <map>

namespace Mlib {

class Players;
struct PhysicsEngineConfig;

struct SupplyDebot {
    FixedArray<double, 3> center;
    const std::map<std::string, uint32_t> supplies;
};

class SupplyDepots: public ISupplyDepots {
public:
    SupplyDepots(Players& players, const PhysicsEngineConfig& cfg);
    ~SupplyDepots();
    void handle_supply_depots();
    virtual void add_supply_depot(
        SceneNode& scene_node,
        const std::map<std::string, uint32_t>& supplies) override;
private:
    Bvh<double, SupplyDebot, 3> bvh_;
    Players& players_;
};

}
