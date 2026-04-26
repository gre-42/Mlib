#include "Load_Scene_Funcs.hpp"
#include <Mlib/Os/Os.hpp>
#include <stdexcept>

using namespace Mlib;

static std::map<std::string, LoadSceneJsonUserFunction>& json_user_functions() {
    static std::map<std::string, LoadSceneJsonUserFunction> result;
    return result;
}

LoadSceneJsonUserFunction* Mlib::LoadSceneFuncs::try_get_json_user_function(const std::string& key) {
    auto& m = json_user_functions();
    auto it = m.find(key);
    if (it == m.end()) {
        return nullptr;
    }
    return &it->second;
}

void Mlib::LoadSceneFuncs::register_json_user_function(std::string key, LoadSceneJsonUserFunction function) {
    if (!json_user_functions().try_emplace(std::move(key), std::move(function)).second) {
        throw std::runtime_error("Multiple functions with name \"" + key + "\" exist");
    }
}
