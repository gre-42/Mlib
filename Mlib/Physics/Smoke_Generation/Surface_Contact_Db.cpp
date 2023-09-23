#include "Surface_Contact_Db.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SurfaceContactDb::SurfaceContactDb() = default;

SurfaceContactDb::~SurfaceContactDb() = default;

void SurfaceContactDb::store_contact_info(
    SurfaceContactInfo info,
    PhysicsMaterial material0,
    PhysicsMaterial material1)
{
    if ((material0 & PhysicsMaterial::SURFACE_BASE_MASK) != material0) {
        THROW_OR_ABORT("Material 0 has modifiers");
    }
    if ((material1 & PhysicsMaterial::SURFACE_BASE_MASK) != material1) {
        THROW_OR_ABORT("Material 1 has modifiers");
    }
    if (!surface_contact_infos_.try_emplace(CommutativeMaterialPair{material0, material1}, std::move(info)).second) {
        THROW_OR_ABORT("Surface contact info already stored for materials");
    }
}

SurfaceContactInfo* SurfaceContactDb::get_contact_info(
    PhysicsMaterial material0,
    PhysicsMaterial material1)
{
    auto sit = surface_contact_infos_.find(CommutativeMaterialPair{
        material0 & PhysicsMaterial::SURFACE_BASE_MASK,
        material1 & PhysicsMaterial::SURFACE_BASE_MASK});
    if (sit == surface_contact_infos_.end()) {
        return nullptr;
    }
    return &sit->second;
}

SurfaceContactInfo* SurfaceContactDb::get_contact_info(const IntersectionScene& c)
{
    PhysicsMaterial material1 = (c.tire_id1 == SIZE_MAX)
        ? c.mesh1_material
        : PhysicsMaterial::SURFACE_BASE_TIRE;
    return get_contact_info(c.mesh0_material, material1);
}
