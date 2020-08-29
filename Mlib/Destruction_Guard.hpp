#pragma once

namespace Mlib {

template <class F>
class DestructionGuard {
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
