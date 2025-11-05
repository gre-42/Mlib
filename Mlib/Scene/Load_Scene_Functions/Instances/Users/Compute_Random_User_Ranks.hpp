#pragma once
namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class ComputeRandomUserRanks {
public:
    static void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
