#pragma once
#include <functional>

namespace Mlib {

struct LoadSceneUserFunctionArgs;

typedef std::function<void(LoadSceneUserFunctionArgs)> LoadSceneUserFunction;

}
