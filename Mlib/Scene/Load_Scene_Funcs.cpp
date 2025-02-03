#include "Load_Scene_Funcs.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

std::map<std::string, LoadSceneJsonUserFunction>& Mlib::LoadSceneFuncs::json_user_functions() {
    static std::map<std::string, LoadSceneJsonUserFunction> result;
    return result;
}

void Mlib::LoadSceneFuncs::register_json_user_function(const std::string& key, LoadSceneJsonUserFunction function) {
    if (!json_user_functions().try_emplace(key, function).second) {
        THROW_OR_ABORT("Multiple functions with name \"" + key + "\" exist");
    }
}
