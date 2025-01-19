#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <map>

namespace Mlib {

namespace LoadSceneFuncs {

void register_json_user_function(const std::string& key, LoadSceneJsonUserFunction function);
std::map<std::string, LoadSceneJsonUserFunction>& json_user_functions();

};

}