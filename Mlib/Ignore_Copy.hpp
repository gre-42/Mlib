#pragma once

namespace Mlib {

template <class T>
struct IgnoreCopy {
    IgnoreCopy() {}
    IgnoreCopy(const IgnoreCopy&) {}
    IgnoreCopy(IgnoreCopy&&) {}
    IgnoreCopy& operator = (const IgnoreCopy&) {}
    T value;
};

}
