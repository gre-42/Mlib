#pragma once
#include <Mlib/Scene/User_Function.hpp>
#include <regex>

namespace Mlib {

class GenInstances {
public:
    static LoadSceneUserFunction user_function;
private:
    static void execute(const std::smatch& match, const LoadSceneUserFunctionArgs& args);
};

}
