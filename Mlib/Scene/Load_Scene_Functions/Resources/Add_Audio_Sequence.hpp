#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>

namespace Mlib {

class AddAudioSequence {
public:
    static LoadSceneJsonUserFunction json_user_function;
private:
    static void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
