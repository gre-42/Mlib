#pragma once
#include <Mlib/Hash.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <Mlib/Std_Hash.hpp>
#include <compare>
#include <string>

namespace Mlib {

using LocalObjectId = uint64_t;

struct RemoteObjectId {
    RemoteSiteId site_id;
    LocalObjectId object_id;
    std::string to_string() const;
    std::string to_displayname() const;
    std::strong_ordering operator <=> (const RemoteObjectId&) const = default;
};

}

template <>
struct std::hash<Mlib::RemoteObjectId>
{
    std::size_t operator() (const Mlib::RemoteObjectId& a) const {
        return Mlib::hash_combine(
            a.site_id,
            a.object_id);
    }
};
