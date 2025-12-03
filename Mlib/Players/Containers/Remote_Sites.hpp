#pragma once
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Map/Verbose_Map.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Value_Unordered_Map.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Memory/Event_Emitter.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <atomic>
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
    NONE = 0,
    LOCAL = 1 << 0,
    REMOTE = 1 << 1,
    INITIAL = 1 << 2,
    LEVEL_LOADING = 1 << 3,
    LEVEL_LOADED = 1 << 4,
    ALL = LOCAL | REMOTE | INITIAL | LEVEL_LOADING | LEVEL_LOADED,
    ALL_LOADED = LOCAL | REMOTE | LEVEL_LOADED,
    ALL_LOCAL = ALL & ~REMOTE,
    ALL_REMOTE = ALL & ~LOCAL,
};

inline bool any(UserTypes types) {
    return types != UserTypes::NONE;
}

inline UserTypes operator & (UserTypes a, UserTypes b) {
    return (UserTypes)((int)a & (int)b);
}

inline UserTypes operator | (UserTypes a, UserTypes b) {
    return (UserTypes)((int)a | (int)b);
}

inline UserTypes& operator |= (UserTypes& a, UserTypes b) {
    return (UserTypes&)((int&)a |= (int)b);
}

enum class UserStatus: uint8_t {
    INITIAL = 0x51,
    LEVEL_LOADING = 0x3F,
    LEVEL_LOADED = 0xC9
};

class RemoteSites;

class UserInfo final: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    UserInfo(
        const DanglingBaseClassRef<RemoteSites>& remote_sites,
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
    UserStatus get_status() const;
    void set_status(UserStatus value);
private:
    std::atomic<UserStatus> status_;
    DanglingBaseClassRef<RemoteSites> remote_sites_;
};

struct SiteInfo {
    SiteInfo();
    ~SiteInfo();
    NonCopyingVector<UserInfo> users;
};

class RemoteSites: public virtual DanglingBaseClass {
    friend UserInfo;
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
    EventEmitter<const UserInfo&> on_user_loaded_level;
    EventEmitter<> on_all_users_loaded_level;
    DanglingBaseClassRef<UserInfo> get_user(RemoteSiteId site_id, uint32_t id);
    DanglingBaseClassRef<const UserInfo> get_user(RemoteSiteId site_id, uint32_t id) const;
    DanglingBaseClassRef<UserInfo> get_user(const VariableAndHash<std::string>& full_name);
    DanglingBaseClassRef<const UserInfo> get_user(const VariableAndHash<std::string>& full_name) const;
    DanglingBaseClassRef<const UserInfo> get_local_user(uint32_t id) const;
    DanglingBaseClassRef<const UserInfo> get_user_by_rank(uint32_t rank) const;
    void print(std::ostream& ostr) const;

    uint32_t compute_random_user_ranks();
    void set_user_status(UserTypes types, UserStatus status);
private:
    SiteInfo& get_site_info(RemoteSiteId site_id);
    void assert_local_users_consistents() const;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    DanglingBaseClassRef<Users> local_users_;
    std::optional<RemoteParams> remote_params_;
    SiteInfo local_site_;
    VerboseMap<RemoteSiteId, SiteInfo> remote_sites_;
    DanglingValueUnorderedMap<VariableAndHash<std::string>, UserInfo> named_users_;
    std::function<void(const UserInfo& user)> on_user_loaded_level_;
    std::function<void()> on_all_users_loaded_level_;
    size_t users_total_;
    size_t users_with_loaded_level_;
};

}
