#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class CreateCheckPoints: public LoadSceneInstanceFunction {
public:
    explicit CreateCheckPoints(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
