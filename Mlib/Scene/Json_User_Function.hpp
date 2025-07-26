#pragma once
#include <functional>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

using LoadSceneJsonUserFunction = std::function<void(LoadSceneJsonUserFunctionArgs)>;

}
