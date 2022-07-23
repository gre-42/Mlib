#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Graph/Interfaces/ISupplyDepots.hpp>
#include <cstdint>
#include <map>

namespace Mlib {

struct PhysicsEngineConfig;

class SupplyDepots: public ISupplyDepots {
public:
    SupplyDepots(const PhysicsEngineConfig& cfg);
    ~SupplyDepots();
    void handle_supply_depots();
    virtual void add_supply_depot(
        const FixedArray<double, 3>& position,
        const std::map<std::string, uint32_t>& supply_depot) override;
private:
    Bvh<double, std::map<std::string, uint32_t>, 3> bvh_;
};

}
