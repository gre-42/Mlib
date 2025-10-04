#pragma once
namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class ForEachUser {
public:
    static void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
