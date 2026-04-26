#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <map>
#include <string>

namespace Mlib {

namespace LoadSceneFuncs {

LoadSceneJsonUserFunction* try_get_json_user_function(const std::string& key);
void register_json_user_function(std::string key, LoadSceneJsonUserFunction function);

};

}