#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <iosfwd>

namespace Mlib {

enum class ObjectCompression: uint32_t;

class IIncrementalObject: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual ~IIncrementalObject() = default;
    virtual void read(std::istream& istr) = 0;
    virtual void write(std::ostream& ostr, ObjectCompression compression) = 0;
};

}
