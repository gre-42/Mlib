#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct LoadSceneJsonUserFunctionArgs;

void invert(const LoadSceneJsonUserFunctionArgs& args);

void invert(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<SceneDir, 3>& rotation);

}
