#include "Permanent_Point_Contact.hpp"
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Containers/Permanent_Contacts.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

PermanentPointContact::PermanentPointContact(
    PermanentContacts& permanent_contacts,
    SceneNode& scene_node0,
    SceneNode& scene_node1,
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const FixedArray<double, 3>& p0,
    const FixedArray<double, 3>& p1)
: permanent_contacts_{permanent_contacts},
  scene_node0_{scene_node0},
  scene_node1_{scene_node1},
  rbp0_{rbp0},
  rbp1_{rbp1},
  p0_{p0},
  p1_{p1}
{
    scene_node0_.destruction_observers.add(*this);
    scene_node1_.destruction_observers.add(*this);
}

void PermanentPointContact::notify_destroyed(const Object& destroyed_object) {
    if (&destroyed_object == &scene_node0_) {
        scene_node1_.destruction_observers.remove(*this);
    }
    if (&destroyed_object == &scene_node1_) {
        scene_node0_.destruction_observers.remove(*this);
    }
    permanent_contacts_.remove(*this);
}

void PermanentPointContact::extend_contact_infos(
    const PhysicsEngineConfig& cfg,
    std::list<std::unique_ptr<ContactInfo>>& contact_infos) {
    auto T0 = rbp0_.abs_transformation();
    auto T1 = rbp1_.abs_transformation();
    contact_infos.push_back(std::make_unique<PointContactInfo2>(
        rbp0_,
        rbp1_,
        PointEqualityConstraint{
            .p0 = T0.transform(p0_),
            .p1 = T1.transform(p1_),
            .beta = cfg.point_equality_beta}));
}