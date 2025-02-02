#pragma once

namespace Mlib {

template <class T>
class IgnoreCopy {
public:
    IgnoreCopy() {}
    IgnoreCopy(const IgnoreCopy&) {}
    IgnoreCopy(IgnoreCopy&&) {}
    IgnoreCopy& operator = (const IgnoreCopy&) { return *this; }
    T& operator * () { return value; }
    const T& operator * () const { return value; }
    T* operator -> () { return &value; }
    const T* operator -> () const { return &value; }
private:
    T value;
};

}
