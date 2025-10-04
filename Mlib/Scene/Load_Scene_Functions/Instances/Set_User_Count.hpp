#pragma once

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class SetUserCount {
public:
    static void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
