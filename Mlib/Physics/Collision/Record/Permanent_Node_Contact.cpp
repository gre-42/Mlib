#include "Permanent_Node_Contact.hpp"
#include <Mlib/Physics/Containers/Permanent_Contacts.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

PermanentNodeContact::PermanentNodeContact(
    PermanentContacts& permanent_contacts,
    DanglingRef<SceneNode> scene_node0,
    DanglingRef<SceneNode> scene_node1,
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1)
    : permanent_contacts_{ permanent_contacts }
    , rbp0_{ rbp0 }
    , rbp1_{ rbp1 }
    , scene_node0_{ scene_node0 }
    , scene_node1_{ scene_node1 }
{
    scene_node0_->destruction_observers.add(*this);
    scene_node1_->destruction_observers.add(*this);
}

void PermanentNodeContact::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    if (destroyed_object.ptr() == scene_node0_.ptr()) {
        scene_node1_->destruction_observers.remove(*this);
    }
    if (destroyed_object.ptr() == scene_node1_.ptr()) {
        scene_node0_->destruction_observers.remove(*this);
    }
    permanent_contacts_.remove(*this);
}
