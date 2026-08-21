#pragma once
#include <Mlib/Scene_Config/Remote_Transmission.hpp>
#include <iosfwd>

namespace Mlib {

class RemoteTransmissionStatistics {
public:
    RemoteTransmissionStatistics();
    ~RemoteTransmissionStatistics();
    void notify_datagram(DatagramIndexType version);
    void print(std::ostream& ostr) const;
private:
    DatagramIndexType old_version_;
    size_t ndropped_;
    size_t nout_of_order_;
};

std::ostream& operator << (std::ostream& ostr, const RemoteTransmissionStatistics& stats);

}
