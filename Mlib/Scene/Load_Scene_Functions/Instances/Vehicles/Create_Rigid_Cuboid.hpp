#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstddef>
#include <optional>
#include <string>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;
class ObjectPool;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
enum class CollidableMode;
class RigidBodyVehicle;

struct CreateRigidCuboidArgs {
    ObjectPool& object_pool;
    const VariableAndHash<std::string>& node;
    const std::string& name;
    const std::string& asset_id;
    float mass;
    FixedArray<float, 3> size;
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

class CreateRigidCuboid: public LoadSceneInstanceFunction {
public:
    explicit CreateRigidCuboid(RenderableScene& renderable_scene);
    RigidBodyVehicle& operator () (const CreateRigidCuboidArgs& args) const;
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
