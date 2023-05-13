#pragma once

namespace Mlib {

template <class F>
class DestructionGuard {
    DestructionGuard(const DestructionGuard&) = delete;
    DestructionGuard& operator = (const DestructionGuard&) = delete;
public:
    explicit DestructionGuard(const F& f)
    : f_{f}
    {}
    ~DestructionGuard() {
        f_();
    }
private:
    F f_;
};

}
