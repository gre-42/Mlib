#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <iosfwd>

namespace Mlib {

class ISendSocket;
enum class SendStatusCode: int;

class FragmentingSender {
public:
    explicit FragmentingSender();
    void send(
        std::istream& istr,
        ISendSocket& socket,
        SendStatusCode& status_code);
private:
    FragmentGroupType group_id_;
};

}
