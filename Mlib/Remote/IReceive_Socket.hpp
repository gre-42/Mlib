#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <iosfwd>

namespace Mlib {

class ISendSocket;

class IReceiveSocket: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual DanglingBaseClassPtr<ISendSocket> try_receive(std::ostream& ostr) = 0;
};

}
