#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Collision/Record/Permanent_Node_Contact.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyVehicle;
class PermanentContacts;

class PermanentPointContact final: public PermanentNodeContact {
public:
    PermanentPointContact(
        PermanentContacts& permanent_contacts,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb0,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb1,
        const FixedArray<ScenePos, 3>& p0,
        const FixedArray<ScenePos, 3>& p1);
    ~PermanentPointContact();
    
    // IPermanentContact
    virtual void extend_contact_infos(
        const PhysicsEngineConfig& cfg,
        const PhysicsPhase& phase,
        std::list<std::unique_ptr<IContactInfo>>& contact_infos) override;
private:
    FixedArray<ScenePos, 3> p0_;
    FixedArray<ScenePos, 3> p1_;
};

}
