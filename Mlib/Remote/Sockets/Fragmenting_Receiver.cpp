#include "Fragmenting_Receiver.hpp"
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Remote/IReceive_Socket.hpp>
#include <vector>

using namespace Mlib;

FragmentingReceiveSocket::FragmentingReceiveSocket() = default;

bool FragmentingReceiveSocket::try_receive(std::ostream& ostr, std::istream& istr) {
    purge_expired_groups();

    // auto magic = read_binary<uint32_t>(istr, "fragment magic", IoVerbosity::SILENT);
    // if (magic != 0xc0febabe) {
    //     throw std::runtime_error("Incorrect fragment magic number");
    // }
    auto group_id = read_binary<FragmentGroupType>(istr, "fragment group ID", IoVerbosity::SILENT);
    auto block_index = read_binary<FragmentIndexType>(istr, "fragment block index", IoVerbosity::SILENT);
    auto nblocks = read_binary<FragmentIndexType>(istr, "fragment nblocks", IoVerbosity::SILENT);

    auto payload = read_all_vector(istr, "dataframe fragment", IoVerbosity::SILENT);

    auto& group = [&]() -> GroupState& {
        auto it = active_groups_.find(group_id);
        if (it == active_groups_.end()) {
            auto it2 = active_groups_.try_emplace(group_id, nblocks);
            if (!it2.second) {
                verbose_abort("Could not insert fragment group ID");
            }
            return it2.first->second;
        } else {
            if (it->second.nblocks != nblocks) {
                throw std::runtime_error("Conflicting number of fragment blocks");
            }
            return it->second;
        }
    }();
    group.arrival_time = std::chrono::steady_clock::now();

    if (block_index >= nblocks) {
        throw std::runtime_error("Malformed packet: block index out of bounds");
    }

    if (!group.blocks.try_emplace(block_index, std::move(payload)).second) {
        throw std::runtime_error((std::stringstream() <<
            "Duplicate packet detected: group " << (group_id + 0) <<
            ", block " << (block_index + 0)).str());
    }
    if (integral_cast<FragmentIndexType>(group.blocks.size()) == group.nblocks) {
        for (FragmentIndexType i = 0; i < group.nblocks; ++i) {
            write_iterable(ostr, group.blocks.at(i), "block data");
        }
        active_groups_.erase(group_id);
        return true;
    }
    return false;
}

void FragmentingReceiveSocket::purge_expired_groups() {
    auto now = std::chrono::steady_clock::now();
    for (auto it = active_groups_.begin(); it != active_groups_.end(); ) {
        if ((now - it->second.arrival_time) > FRAGMENT_TIMEOUT) {
            it = active_groups_.erase(it);
        } else {
            ++it;
        }
    }
}
