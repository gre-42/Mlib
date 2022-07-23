#include "Supply_Depots.hpp"
#include <Mlib/Physics/Physics_Engine_Config.hpp>

using namespace Mlib;

SupplyDepots::SupplyDepots(const PhysicsEngineConfig& cfg)
: bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, 7}
{}

SupplyDepots::~SupplyDepots()
{}

void SupplyDepots::handle_supply_depots() {

}

void SupplyDepots::add_supply_depot(
    const FixedArray<double, 3>& position,
    const std::map<std::string, uint32_t>& supplies)
{
    bvh_.insert(AxisAlignedBoundingBox<double, 3>{position}, supplies);
}
