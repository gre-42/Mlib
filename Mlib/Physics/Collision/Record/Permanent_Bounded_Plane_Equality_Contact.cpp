#include "Permanent_Bounded_Plane_Equality_Contact.hpp"
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Containers/Permanent_Contacts.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Config/Physics_Engine_Config.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

PermanentBoundedPlaneEqualityContact::PermanentBoundedPlaneEqualityContact(
    PermanentContacts& permanent_contacts,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb0,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb1,
    const FixedArray<ScenePos, 3>& p0,
    const FixedArray<ScenePos, 3>& p1,
    const FixedArray<float, 3>& normal0)
    : PermanentNodeContact{
        permanent_contacts,
        rb0,
        rb1
    }
    , p0_{ p0 }
    , p1_{ p1 }
    , normal0_{ normal0 }
{}

PermanentBoundedPlaneEqualityContact::~PermanentBoundedPlaneEqualityContact() {
    on_destroy.clear();
}

void PermanentBoundedPlaneEqualityContact::extend_contact_infos(
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    std::list<std::unique_ptr<IContactInfo>>& contact_infos)
{
    if (!is_in_group(phase)) {
        return;
    }
    auto T0 = rb0_->rbp_.abs_transformation();
    auto T1 = rb1_->rbp_.abs_transformation();
    contact_infos.push_back(std::make_unique<PlaneContactInfo2>(
        rb0_->rbp_,
        rb1_->rbp_,
        BoundedPlaneEqualityConstraint{
            PlaneEqualityConstraint{
                .pec = PointEqualityConstraint{
                    .p0 = T0.transform(p0_),
                    .p1 = T1.transform(p1_),
                    .beta = cfg.point_equality_beta
                },
                .plane_normal = T0.rotate(normal0_)
            } }));
}
