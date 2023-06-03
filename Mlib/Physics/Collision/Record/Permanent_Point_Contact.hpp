#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Collision/Record/IPermanent_Contact.hpp>

namespace Mlib {

class RigidBodyPulses;
class PermanentContacts;
class SceneNode;

class PermanentPointContact: public IPermanentContact, public DestructionObserver {
public:
    PermanentPointContact(
        PermanentContacts& permanent_contacts,
        SceneNode& scene_node0,
        SceneNode& scene_node1,
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const FixedArray<double, 3>& p0,
        const FixedArray<double, 3>& p1);
    
    // IPermanentContact
    virtual void extend_contact_infos(
        const PhysicsEngineConfig& cfg,
        std::list<std::unique_ptr<ContactInfo>>& contact_infos) override;
    
    // DestructionObserver
    virtual void notify_destroyed(const Object& destroyed_object) override;
private:
    PermanentContacts& permanent_contacts_;
    SceneNode& scene_node0_;
    SceneNode& scene_node1_;
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
    FixedArray<double, 3> p0_;
    FixedArray<double, 3> p1_;
};

}
