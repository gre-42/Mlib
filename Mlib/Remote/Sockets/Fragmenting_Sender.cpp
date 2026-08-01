#include "Fragmenting_Sender.hpp"
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>
#include <Mlib/Remote/Send_Status_Code.hpp>
#include <sstream>
#include <vector>

using namespace Mlib;

FragmentingSender::FragmentingSender()
    : group_id_{ 0 }
{}

void FragmentingSender::send(
    std::istream& istr,
    ISendSocket& socket,
    SendStatusCode& status_code)
{
    auto begin = istr.tellg();
    istr.seekg(0, std::ios::end);
    auto len = integral_cast<size_t>(istr.tellg() - begin);
    istr.seekg(begin);
    if (len == 0) {
        throw std::runtime_error("Attempt to send empty datagram");
    }
    std::vector<char> buffer(MAX_FRAGMENT_BYTES);
    auto nblocks = integral_cast<FragmentIndexType>((len - 1) / MAX_FRAGMENT_BYTES + 1);
    for (FragmentIndexType block_index = 0; block_index < nblocks; ++block_index) {
        buffer.resize(std::min(MAX_FRAGMENT_BYTES, len));
        read_vector(istr, buffer, "payload", IoVerbosity::SILENT);
        std::stringstream sstr;
        // write_binary<uint32_t>(sstr, 0xc0febabe, "fragment magic");
        write_binary(sstr, group_id_, "fragment group");
        write_binary(sstr, block_index, "fragment block_index");
        write_binary(sstr, nblocks, "fragment nblocks");
        write_iterable(sstr, buffer, "payload");

        socket.send(sstr, status_code);
        if (status_code != SendStatusCode::SUCCESS) {
            break;
        }
        len -= MAX_FRAGMENT_BYTES;
    }
    ++group_id_;
}
