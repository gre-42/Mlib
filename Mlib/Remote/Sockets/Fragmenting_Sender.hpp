#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <iosfwd>

namespace Mlib {

class ISendSocket;

class FragmentingSender {
public:
    explicit FragmentingSender();
    void send(std::istream& istr, ISendSocket& socket);
private:
    FragmentGroupType group_id_;
};

}
