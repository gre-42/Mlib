#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct AnimationState;

class AnimationStateUpdater {
public:
    virtual ~AnimationStateUpdater() = default;
    virtual void notify_movement_intent() = 0;
    virtual void update_animation_state(AnimationState* animation_state) = 0;
};

}
