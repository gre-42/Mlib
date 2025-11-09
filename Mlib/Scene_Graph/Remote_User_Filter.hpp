#pragma once
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <compare>
#include <cstddef>
#include <optional>

namespace Mlib {

class RemoteObserver;

class ViewableRemoteObject {
    friend RemoteObserver;
public:
    static inline ViewableRemoteObject all() {
        return {};
    }
    ViewableRemoteObject() = default;
    ViewableRemoteObject(
        const std::optional<RemoteSiteId>& site_id,
        const std::optional<uint32_t>& user_id)
        : site_id_{ site_id }
        , user_id_{ user_id }
    {}
private:
    std::optional<RemoteSiteId> site_id_;
    std::optional<uint32_t> user_id_;
};

class RemoteObserver {
public:
    static inline RemoteObserver all() {
        return {};
    }
    RemoteObserver() = default;
    RemoteObserver(
        const std::optional<RemoteSiteId>& site_id,
        const std::optional<uint32_t>& user_id)
        : site_id_{ site_id }
        , user_id_{ user_id }
    {}
    inline bool can_see(const ViewableRemoteObject& obj) const;
    std::strong_ordering operator <=> (const RemoteObserver&) const = default;
private:
    std::optional<RemoteSiteId> site_id_;
    std::optional<uint32_t> user_id_;
};

inline bool RemoteObserver::can_see(const ViewableRemoteObject& obj) const
{
    if (site_id_.has_value() &&
        obj.site_id_.has_value() &&
        (*site_id_ != *obj.site_id_))
    {
        return false;
    }
    if (user_id_.has_value() &&
        obj.user_id_.has_value() &&
        (*user_id_ != *obj.user_id_))
    {
        return false;
    }
    return true;
}

}
