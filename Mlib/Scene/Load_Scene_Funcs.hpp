#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <map>
#include <string>

namespace Mlib {

namespace LoadSceneFuncs {

std::map<std::string, LoadSceneJsonUserFunction>& json_user_functions();
void register_json_user_function(std::string key, LoadSceneJsonUserFunction function);

};

}