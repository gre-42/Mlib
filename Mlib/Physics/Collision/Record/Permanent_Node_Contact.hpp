#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Physics/Collision/Record/IPermanent_Contact.hpp>

namespace Mlib {

class RigidBodyVehicle;
class PermanentContacts;
struct PhysicsPhase;

class PermanentNodeContact: public IPermanentContact, public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    PermanentNodeContact(
        PermanentContacts& permanent_contacts,
        const DanglingBaseClassRef<RigidBodyVehicle>& rbp0,
        const DanglingBaseClassRef<RigidBodyVehicle>& rbp1);
    ~PermanentNodeContact();
protected:
    bool is_in_group(const PhysicsPhase& phase) const;
    DanglingBaseClassPtr<RigidBodyVehicle> rb0_;
    DanglingBaseClassPtr<RigidBodyVehicle> rb1_;
private:
    PermanentContacts& permanent_contacts_;
    DestructionFunctionsRemovalTokens on_destroy_rb0_;
    DestructionFunctionsRemovalTokens on_destroy_rb1_;
};

}
