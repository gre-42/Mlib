#pragma once
#include <Mlib/Scene/Load_Scene_User_Function.hpp>
#include <string>

namespace Mlib {

class JsonMacroArguments;

class AppendFocuses {
public:
    static LoadSceneUserFunction user_function;
    static const std::string key;
private:
    static void execute(const JsonMacroArguments& json_macro_arguments, const LoadSceneUserFunctionArgs& args);
};

}
