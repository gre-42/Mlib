#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Collision/Record/Permanent_Node_Contact.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyPulses;
class PermanentContacts;

class PermanentBoundedPlaneEqualityContact final: public PermanentNodeContact {
public:
    PermanentBoundedPlaneEqualityContact(
        PermanentContacts& permanent_contacts,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb0,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb1,
        const FixedArray<ScenePos, 3>& p0,
        const FixedArray<ScenePos, 3>& p1,
        const FixedArray<float, 3>& normal0);
    virtual ~PermanentBoundedPlaneEqualityContact() override;

    // IPermanentContact
    virtual void extend_contact_infos(
        const PhysicsEngineConfig& cfg,
        const PhysicsPhase& phase,
        std::list<std::unique_ptr<IContactInfo>>& contact_infos) override;
private:
    FixedArray<ScenePos, 3> p0_;
    FixedArray<ScenePos, 3> p1_;
    FixedArray<float, 3> normal0_;
};

}
