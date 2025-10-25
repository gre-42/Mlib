#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <iosfwd>

namespace Mlib {

class ISendSocket: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual void send(std::istream& istr) = 0;
};

}
