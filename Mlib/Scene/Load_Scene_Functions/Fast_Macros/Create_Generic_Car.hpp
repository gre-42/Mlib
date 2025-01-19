#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>

namespace Mlib {

class CreateGenericCar: public LoadSceneInstanceFunction {
public:
    static LoadSceneJsonUserFunction json_user_function;
    static const std::string key;
    explicit CreateGenericCar(RenderableScene& renderable_scene);
private:
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
