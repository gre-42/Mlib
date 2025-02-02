#pragma once

namespace Mlib {

template <class TCounter>
class IncrementInDestructor {
public:
    IncrementInDestructor(TCounter& counter)
    : counter_{counter}
    {}
    ~IncrementInDestructor() {
        ++counter_;
    }
private:
    TCounter& counter_;
};

}
