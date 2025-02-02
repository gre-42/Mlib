#pragma once
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function.hpp>

namespace Mlib {

class Globals {
public:
    static LoadSceneJsonUserFunction json_user_function;
    static const std::string key;
};

}
