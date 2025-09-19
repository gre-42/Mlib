#include "Permanent_Node_Contact.hpp"
#include <Mlib/Physics/Containers/Permanent_Contacts.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

PermanentNodeContact::PermanentNodeContact(
    PermanentContacts& permanent_contacts,
    DanglingBaseClassRef<SceneNode> scene_node0,
    DanglingBaseClassRef<SceneNode> scene_node1,
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1)
    : rbp0_{ rbp0 }
    , rbp1_{ rbp1 }
    , permanent_contacts_{ permanent_contacts }
    , scene_node0_{ scene_node0 }
    , scene_node1_{ scene_node1 }
{
    scene_node0_->destruction_observers.add({ *this, CURRENT_SOURCE_LOCATION });
    scene_node1_->destruction_observers.add({ *this, CURRENT_SOURCE_LOCATION });
}

void PermanentNodeContact::notify_destroyed(SceneNode& destroyed_object) {
    if (&destroyed_object == &scene_node0_.get()) {
        scene_node1_->destruction_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
    }
    if (&destroyed_object == &scene_node1_.get()) {
        scene_node0_->destruction_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
    }
    permanent_contacts_.remove(*this);
}
