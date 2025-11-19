#pragma once
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Map/Verbose_Map.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Value_Unordered_Map.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <map>
#include <optional>

namespace Mlib {

class Users;

enum class UserType {
    LOCAL,
    REMOTE
};

enum class UserTypes {
    LOCAL,
    ALL
};

struct UserInfo final: public virtual DestructionNotifier, public virtual DanglingBaseClass {
    UserInfo(
        const std::optional<RemoteSiteId>& site_id,
        uint32_t user_id,
        std::string name,
        std::string full_name,
        UserType type);
    virtual ~UserInfo() override;
    std::optional<RemoteSiteId> site_id;
    uint32_t user_id;
    std::string name;
    std::string full_name;
    UserType type;
    uint32_t random_rank;
};

struct SiteInfo {
    SiteInfo();
    ~SiteInfo();
    NonCopyingVector<UserInfo> users;
};

class RemoteSites: public virtual DanglingBaseClass {
public:
    explicit RemoteSites(
        const DanglingBaseClassRef<Users>& local_users,
        const std::optional<RemoteParams>& remote_params);
    ~RemoteSites();
    std::optional<RemoteSiteId> get_local_site_id() const;
    uint32_t get_local_user_count() const;
    uint32_t get_user_count(RemoteSiteId site_id) const;
    uint32_t get_total_user_count(UserTypes user_types) const;
    void set_local_user_count(uint32_t user_count);
    void set_user_count(RemoteSiteId site_id, uint32_t user_count);
    bool for_each_site_user(
        const std::function<bool(UserInfo& user)>& operation,
        UserTypes user_types);
    bool for_each_site_user(
        const std::function<bool(const UserInfo& user)>& operation,
        UserTypes user_types) const;
    DanglingBaseClassRef<const UserInfo> get_user(const VariableAndHash<std::string>& full_name) const;
    DanglingBaseClassRef<const UserInfo> get_local_user(uint32_t id) const;
    DanglingBaseClassRef<const UserInfo> get_user_by_rank(uint32_t rank) const;
    void print(std::ostream& ostr) const;

    uint32_t compute_random_user_ranks();
private:
    void assert_local_users_consistents() const;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    DanglingBaseClassRef<Users> local_users_;
    std::optional<RemoteParams> remote_params_;
    SiteInfo local_site_;
    VerboseMap<RemoteSiteId, SiteInfo> remote_sites_;
    DanglingValueUnorderedMap<VariableAndHash<std::string>, UserInfo> named_users_;
};

}
