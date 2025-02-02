#include "Permanent_Line_Contact.hpp"
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Containers/Permanent_Contacts.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

PermanentLineContact::PermanentLineContact(
    PermanentContacts& permanent_contacts,
    DanglingRef<SceneNode> scene_node0,
    DanglingRef<SceneNode> scene_node1,
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const FixedArray<ScenePos, 3>& p0,
    const FixedArray<ScenePos, 3>& p1,
    const FixedArray<float, 3>& line0)
    : PermanentNodeContact{
        permanent_contacts,
        scene_node0,
        scene_node1,
        rbp0,
        rbp1
    }
    , p0_{ p0 }
    , p1_{ p1 }
    , line0_{ line0 }
{}

void PermanentLineContact::extend_contact_infos(
    const PhysicsEngineConfig& cfg,
    std::list<std::unique_ptr<IContactInfo>>& contact_infos)
{
    auto T0 = rbp0_.abs_transformation();
    auto T1 = rbp1_.abs_transformation();
    contact_infos.push_back(std::make_unique<LineContactInfo2>(
        rbp0_,
        rbp1_,
        LineEqualityConstraint{
            .pec = PointEqualityConstraint{
                .p0 = T0.transform(p0_),
                .p1 = T1.transform(p1_),
                .beta = cfg.point_equality_beta
            },
            .null_space = T0.rotate(line0_)
        }));
}
