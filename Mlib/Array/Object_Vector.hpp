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
    using BaseVector::BaseVector;
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
