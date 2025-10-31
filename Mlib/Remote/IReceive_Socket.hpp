#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <iosfwd>
#include <memory>

namespace Mlib {

class ISendSocket;

class IReceiveSocket: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual std::unique_ptr<ISendSocket> try_receive(std::ostream& ostr) = 0;
};

}
