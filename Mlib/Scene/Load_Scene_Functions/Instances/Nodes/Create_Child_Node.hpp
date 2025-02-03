#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Graph/Pose_Interpolation_Mode.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class CreateChildNode: public LoadSceneInstanceFunction {
public:
    explicit CreateChildNode(RenderableScene& renderable_scene);
    void operator () (
        const std::string& type,
        const std::string& parent,
        const std::string& name,
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation = fixed_zeros<float, 3>(),
        float scale = 1.f,
        PoseInterpolationMode interpolation = PoseInterpolationMode::ENABLED) const;
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
