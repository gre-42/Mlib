#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class Spawn: public LoadSceneInstanceFunction {
public:
    explicit Spawn(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
