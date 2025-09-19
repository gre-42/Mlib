#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Collision/Record/Permanent_Node_Contact.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyPulses;
class PermanentContacts;
class SceneNode;

class PermanentBoundedPlaneEqualityContact: public PermanentNodeContact {
public:
    PermanentBoundedPlaneEqualityContact(
        PermanentContacts& permanent_contacts,
        DanglingBaseClassRef<SceneNode> scene_node0,
        DanglingBaseClassRef<SceneNode> scene_node1,
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const FixedArray<ScenePos, 3>& p0,
        const FixedArray<ScenePos, 3>& p1,
        const FixedArray<float, 3>& normal0);
    
    // IPermanentContact
    virtual void extend_contact_infos(
        const PhysicsEngineConfig& cfg,
        std::list<std::unique_ptr<IContactInfo>>& contact_infos) override;
private:
    FixedArray<ScenePos, 3> p0_;
    FixedArray<ScenePos, 3> p1_;
    FixedArray<float, 3> normal0_;
};

}
