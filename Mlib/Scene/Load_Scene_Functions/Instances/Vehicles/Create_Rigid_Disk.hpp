#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Graph/Resources/Physics_Resource_Filter.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <optional>
#include <string>

namespace Mlib {

class ObjectPool;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
enum class CollidableMode;
class RigidBodyVehicle;

struct CreateRigidDiskArgs {
    ObjectPool& object_pool;
    const std::string& node;
    const std::string& name;
    const std::string& asset_id;
    float mass;
    float radius;
    const FixedArray<float, 3> com;
    const FixedArray<float, 3> v;
    const FixedArray<float, 3> w;
    const FixedArray<float, 3> I_rotation;
    const TransformationMatrix<double, double, 3>* geographic_coordinates;
    RigidBodyVehicleFlags flags = RigidBodyVehicleFlags::NONE;
    CompressedScenePos waypoint_dy = (CompressedScenePos)0.f;
    std::optional<std::string> hitboxes;
    PhysicsResourceFilter hitbox_filter;
    CollidableMode collidable_mode;
};

class CreateRigidDisk: public LoadSceneInstanceFunction {
public:
    static LoadSceneJsonUserFunction json_user_function;
    static const std::string key;
    explicit CreateRigidDisk(RenderableScene& renderable_scene);
    RigidBodyVehicle& operator () (const CreateRigidDiskArgs& args) const;
private:
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
