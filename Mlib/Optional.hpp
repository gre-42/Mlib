#pragma once

namespace Mlib {

enum class OptionalState {
    NONE,
    SOME
};

template <class T>
class Optional {
    Optional& operator = (const Optional&) = delete;
public:
    template <typename... Args>
    explicit Optional(OptionalState state, Args&&... args)
    : state_{state}
    {
        if (state_ == OptionalState::SOME) {
            new (t_) T(std::forward<Args>(args)...);
        }
    }
    ~Optional() {
        if (state_ == OptionalState::SOME) {
            ((T*)t_)->~T();
        }
    }
private:
    char t_[sizeof(T)];
    OptionalState state_;
};

}
