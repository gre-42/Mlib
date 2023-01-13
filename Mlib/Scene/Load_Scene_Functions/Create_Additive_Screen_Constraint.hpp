#pragma once
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function.hpp>

namespace Mlib {

class CreateAdditiveScreenConstraint {
public:
    static LoadSceneUserFunction user_function;
private:
    static void execute(const Mlib::re::smatch& match, const LoadSceneUserFunctionArgs& args);
};

}
