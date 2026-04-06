#include "Permanent_Node_Contact.hpp"
#include <Mlib/Physics/Containers/Collision_Group.hpp>
#include <Mlib/Physics/Containers/Permanent_Contacts.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

PermanentNodeContact::PermanentNodeContact(
    PermanentContacts& permanent_contacts,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb0,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb1)
    : rb0_{ rb0.ptr() }
    , rb1_{ rb1.ptr() }
    , permanent_contacts_{ permanent_contacts }
    , on_destroy_rb0_{ rb0->on_destroy.deflt, CURRENT_SOURCE_LOCATION }
    , on_destroy_rb1_{ rb1->on_destroy.deflt, CURRENT_SOURCE_LOCATION }
{
    on_destroy_rb0_.add([this](){ permanent_contacts_.remove(*this); }, CURRENT_SOURCE_LOCATION);
    on_destroy_rb1_.add([this](){ permanent_contacts_.remove(*this); }, CURRENT_SOURCE_LOCATION);
    rb0_->permanent_colliders_.emplace({*this, CURRENT_SOURCE_LOCATION}, CURRENT_SOURCE_LOCATION);
    rb1_->permanent_colliders_.emplace({*this, CURRENT_SOURCE_LOCATION}, CURRENT_SOURCE_LOCATION);
}

PermanentNodeContact::~PermanentNodeContact() {
    rb0_ = nullptr;
    rb1_ = nullptr;
    if (!on_destroy.empty()) {
        verbose_abort("PermanentNodeContact::on_destroy must be called by derived classes");
    }
}

bool PermanentNodeContact::is_in_group(const PhysicsPhase& phase) const {
    if (phase.group.penetration_class == PenetrationClass::BULLET_LINE) {
        return false;
    }
    auto c0 = phase.group.rigid_bodies.contains(&rb0_->rbp_);
    auto c1 = phase.group.rigid_bodies.contains(&rb1_->rbp_);
    if (c0 != c1) {
        throw std::runtime_error("Inconsistent collision phase");
    }
    return c0;
}
