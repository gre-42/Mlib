#pragma once
#include <Mlib/Remote/IReceive_Socket.hpp>
#include <Mlib/Scene_Config/Remote_Transmission.hpp>
#include <chrono>
#include <cstddef>
#include <unordered_map>

namespace Mlib {

class FragmentingReceiveSocket {
public:
    explicit FragmentingReceiveSocket();
    bool try_receive(std::ostream& ostr, std::istream& istr);
private:
    void purge_expired_groups();
    struct GroupState {
        FragmentIndexType nblocks;
        std::chrono::steady_clock::time_point arrival_time;
        std::unordered_map<FragmentIndexType, std::vector<std::byte>> blocks;
    };
    std::unordered_map<FragmentGroupType, GroupState> active_groups_;
};

}
