#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <cstddef>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct AnimationState;

class AnimationStateUpdater: public virtual DanglingBaseClass {
public:
    virtual ~AnimationStateUpdater() = default;
    virtual void notify_movement_intent() = 0;
    virtual std::unique_ptr<AnimationState> update_animation_state(
        const AnimationState& animation_state) = 0;
};

}
