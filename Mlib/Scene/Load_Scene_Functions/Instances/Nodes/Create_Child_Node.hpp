#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Pose_Interpolation_Mode.hpp>
#include <cstddef>

namespace Mlib {

template <class T>
class VariableAndHash;
struct LoadSceneJsonUserFunctionArgs;

class CreateChildNode: public LoadPhysicsSceneInstanceFunction {
public:
    explicit CreateChildNode(PhysicsScene& physics_scene);
    void operator () (
        const std::string& type,
        const VariableAndHash<std::string>& parent,
        const VariableAndHash<std::string>& name,
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation = fixed_zeros<float, 3>(),
        float scale = 1.f,
        PoseInterpolationMode interpolation = PoseInterpolationMode::ENABLED) const;
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
