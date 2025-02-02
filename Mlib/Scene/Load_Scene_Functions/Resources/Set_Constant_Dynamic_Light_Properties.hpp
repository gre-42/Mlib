#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <string>

namespace Mlib {

class SetConstantDynamicLightProperties {
public:
    static const std::string key;
    static LoadSceneJsonUserFunction json_user_function;
};

}
