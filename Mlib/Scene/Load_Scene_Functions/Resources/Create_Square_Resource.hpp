#pragma once
#include <Mlib/Scene/User_Function.hpp>
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

class CreateSquareResource {
public:
    static LoadSceneUserFunction user_function;
private:
    static void execute(const Mlib::re::smatch& match, const LoadSceneUserFunctionArgs& args);
};

}
