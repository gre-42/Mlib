#pragma once
#include <Mlib/Misc/Object.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <vector>

namespace Mlib {

template <class TValue>
class ObjectVector: public std::vector<TValue>, public virtual Object
{
public:
    using BaseVector = std::vector<TValue>;
    // Not supported by MSVC: error C2468: constructor or destructor cannot be
    // 'constexpr' in a class or struct with virtual base classes
    // using BaseVector::BaseVector;
    explicit ObjectVector(std::size_t size): BaseVector(size) {}
    BaseVector& elements() {
        return *this;
    }
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(elements());
    }
};

}
