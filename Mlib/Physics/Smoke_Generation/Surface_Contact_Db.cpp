#include "Surface_Contact_Db.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SurfaceContactDb::SurfaceContactDb() = default;

SurfaceContactDb::~SurfaceContactDb() = default;

void SurfaceContactDb::store_contact_info(
    const SurfaceContactInfo& info,
    PhysicsMaterial material0,
    PhysicsMaterial material1)
{
    if (!surface_contact_infos_.insert({UnorderedMaterialPair{material0, material1}, info}).second) {
        THROW_OR_ABORT("Surface contact info already store for materials");
    }
}

SurfaceContactInfo* SurfaceContactDb::get_contact_info(
    const FixedArray<double, 3>& intersection_point,
    const IntersectionScene& c)
{
    PhysicsMaterial material1 = (c.tire_id1 == SIZE_MAX)
        ? c.mesh1_material
        : PhysicsMaterial::SURFACE_BASE_TIRE;
    auto sit = surface_contact_infos_.find(UnorderedMaterialPair{
        c.mesh0_material & PhysicsMaterial::SURFACE_BASE_MASK,
        material1 & PhysicsMaterial::SURFACE_BASE_MASK});
    if (sit == surface_contact_infos_.end()) {
        return nullptr;
    }
    return &sit->second;
}
