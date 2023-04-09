#pragma once
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene/User_Function.hpp>

namespace Mlib {

class RespawnAllPlayers: public LoadSceneInstanceFunction {
public:
    static LoadSceneUserFunction user_function;
    static const std::string key;
private:
    explicit RespawnAllPlayers(RenderableScene& renderable_scene);
    void execute(const Mlib::re::smatch& match, const LoadSceneUserFunctionArgs& args);
};

}
