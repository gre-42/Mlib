#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstddef>
#include <optional>
#include <string>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
enum class CollidableMode;
class RigidBodyVehicle;

struct CreateRigidDiskArgs {
    const VariableAndHash<std::string>& node;
    const std::string& name;
    const std::string& asset_id;
    float mass;
    float radius;
    FixedArray<float, 3> com;
    FixedArray<float, 3> v;
    FixedArray<float, 3> w;
    FixedArray<float, 3> I_rotation;
    bool with_penetration_limits;
    const TransformationMatrix<double, double, 3>* geographic_coordinates;
    RigidBodyVehicleFlags flags = RigidBodyVehicleFlags::NONE;
    CompressedScenePos waypoint_dy = (CompressedScenePos)0.f;
    std::optional<VariableAndHash<std::string>> hitboxes;
    ColoredVertexArrayFilter hitbox_filter;
    CollidableMode collidable_mode;
};

class CreateRigidDisk: public LoadPhysicsSceneInstanceFunction {
public:
    explicit CreateRigidDisk(PhysicsScene& physics_scene);
    RigidBodyVehicle& operator () (const CreateRigidDiskArgs& args) const;
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
