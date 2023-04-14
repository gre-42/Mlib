#pragma once
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function.hpp>

namespace Mlib {

class GenContourEdges {
public:
    static LoadSceneUserFunction user_function;
    static const std::string key;
private:
    static void execute(const Mlib::re::smatch& match, const LoadSceneUserFunctionArgs& args);
};

}
