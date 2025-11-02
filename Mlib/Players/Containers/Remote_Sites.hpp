#pragma once
#include <Mlib/Map/Verbose_Map.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <map>
#include <optional>

namespace Mlib {

class Users;

class RemoteSites: public virtual DanglingBaseClass {
public:
    explicit RemoteSites(
        const DanglingBaseClassRef<Users>& local_users,
        const std::optional<RemoteParams>& remote_params);
    ~RemoteSites();
    uint32_t get_user_count(RemoteSiteId site_id) const;
    void set_user_count(RemoteSiteId site_id, uint32_t user_count);
    void for_each_site_user(const std::function<void(RemoteSiteId site_id, uint32_t)>& operation) const;
    void print(std::ostream& ostr) const;
private:
    mutable SafeAtomicSharedMutex mutex_;
    DanglingBaseClassRef<Users> local_users_;
    std::optional<RemoteParams> remote_params_;
    VerboseMap<RemoteSiteId, uint32_t> sites_;
};

}
