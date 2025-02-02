#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Collision/Record/IPermanent_Contact.hpp>

namespace Mlib {

class RigidBodyPulses;
class PermanentContacts;
class SceneNode;

class PermanentNodeContact: public IPermanentContact, public DestructionObserver<SceneNode&>, public virtual DanglingBaseClass {
public:
    PermanentNodeContact(
        PermanentContacts& permanent_contacts,
        DanglingRef<SceneNode> scene_node0,
        DanglingRef<SceneNode> scene_node1,
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1);
    
    // DestructionObserver
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
protected:
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
private:
    PermanentContacts& permanent_contacts_;
    DanglingRef<SceneNode> scene_node0_;
    DanglingRef<SceneNode> scene_node1_;
};

}
