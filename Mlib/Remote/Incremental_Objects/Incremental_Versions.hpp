#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <iosfwd>

namespace Mlib {

struct IncrementalVersionsRead {
    DatagramIndexType local_remote_version;
    DatagramIndexType remote_base_version;
    DatagramIndexType remote_new_version;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(local_remote_version);
        archive(remote_base_version);
        archive(remote_new_version);
    }
};

struct IncrementalVersionsWrite {
    DatagramIndexType remote_local_version;
    DatagramIndexType local_base_version;
    DatagramIndexType local_new_version;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(remote_local_version);
        archive(local_base_version);
        archive(local_new_version);
    }
};

struct SiteVersions {
    DatagramIndexType local_version = 0;
    DatagramIndexType remote_version = 0;
};

struct SocketVersions {
    SiteVersions local;
    DatagramIndexType remote_version;
};

std::ostream& operator << (std::ostream& ostr, const IncrementalVersionsRead& versions);
std::ostream& operator << (std::ostream& ostr, const IncrementalVersionsWrite& versions);

}
