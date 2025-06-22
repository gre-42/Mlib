#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

class CreateAcousticPressureSubdomain: public LoadRenderableSceneInstanceFunction {
public:
    explicit CreateAcousticPressureSubdomain(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
