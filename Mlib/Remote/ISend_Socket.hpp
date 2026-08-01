#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <iosfwd>

namespace Mlib {

enum class SendStatusCode: int;

class ISendSocket: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual ~ISendSocket() = default;
    virtual void send(std::istream& istr, SendStatusCode& status_code) = 0;
};

}
