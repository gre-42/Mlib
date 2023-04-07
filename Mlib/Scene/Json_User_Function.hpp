#pragma once
#include <functional>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

typedef std::function<void(LoadSceneJsonUserFunctionArgs)> LoadSceneJsonUserFunction;

}
