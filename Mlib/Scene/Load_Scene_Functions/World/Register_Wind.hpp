#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>

namespace Mlib {

class RegisterWind {
public:
    static LoadSceneJsonUserFunction json_user_function;
    static const std::string key;
};

}
