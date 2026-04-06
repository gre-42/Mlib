#include "Load_Scene_Funcs.hpp"
#include <stdexcept>

using namespace Mlib;

std::map<std::string, LoadSceneJsonUserFunction>& Mlib::LoadSceneFuncs::json_user_functions() {
    static std::map<std::string, LoadSceneJsonUserFunction> result;
    return result;
}

void Mlib::LoadSceneFuncs::register_json_user_function(std::string key, LoadSceneJsonUserFunction function) {
    if (!json_user_functions().try_emplace(std::move(key), std::move(function)).second) {
        throw std::runtime_error("Multiple functions with name \"" + key + "\" exist");
    }
}
