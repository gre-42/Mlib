#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <iosfwd>

namespace Mlib {

class IIncrementalObject;

class IIncrementalObjectFactory: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual DanglingBaseClassPtr<IIncrementalObject> try_create_shared_object(std::istream& istr) = 0;
};

}
