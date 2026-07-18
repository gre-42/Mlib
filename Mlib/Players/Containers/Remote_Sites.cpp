#include "Remote_Sites.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Players/Containers/Users.hpp>
#include <Mlib/Stats/Arange.hpp>
#include <mutex>
#include <random>
#include <stdexcept>

using namespace Mlib;

UserInfo::UserInfo(
    const DanglingBaseClassRef<RemoteSites>& remote_sites,
    const std::optional<RemoteSiteId>& site_id,
    NUserCountType user_id,
    std::string name,
    std::string full_name,
    UserType type)
    : site_id{ site_id }
    , user_id{ user_id }
    , name{ std::move(name) }
    , full_name{ std::move(full_name) }
    , type{ type }
    , random_rank{ remote_sites->compute_free_user_rank() }
    , status_{ UserStatus::INITIAL }
    , remote_sites_{ remote_sites }
{
    if (remote_sites_->users_total_ != remote_sites_->named_users_.size()) {
        verbose_abort((std::stringstream() << "Inconsistent user count in ctor: " <<
            remote_sites_->users_total_ << " != " << remote_sites_->named_users_.size()).str());
    }
    ++remote_sites_->users_total_;
}

UserInfo::~UserInfo() {
    --remote_sites_->users_total_;
    on_destroy.clear();
    if (remote_sites_->users_total_ != remote_sites_->named_users_.size()) {
        verbose_abort((std::stringstream() << "Inconsistent user count in dtor: " <<
            remote_sites_->users_total_ << " != " << remote_sites_->named_users_.size()).str());
    }
}

UserStatus UserInfo::get_status() const {
    return status_;
}

void UserInfo::set_status(UserStatus status) {
    if ((status != UserStatus::INITIAL) &&
        (status != UserStatus::LEVEL_LOADED) &&
        (status != UserStatus::LEVEL_LOADING))
    {
        throw std::runtime_error("Unknown user status");
    }
    auto old_status = status_.load();
    status_ = status;
    #define status DO_NOT_USE_ME
    if (old_status != UserStatus::LEVEL_LOADED) {
        if (status_ == UserStatus::LEVEL_LOADED) {
            ++remote_sites_->users_with_loaded_level_;
            remote_sites_->on_user_loaded_level.emit(*this);
            if (remote_sites_->users_with_loaded_level_ == remote_sites_->users_total()) {
                remote_sites_->on_all_users_loaded_level.emit();
                remote_sites_->on_all_users_loaded_level.clear();
            }
        }
    } else {
        if (status_ != UserStatus::LEVEL_LOADED) {
            --remote_sites_->users_with_loaded_level_;
        }
    }
    #undef status
}

std::ostream& Mlib::operator << (std::ostream& ostr, const UserInfo& user_info) {
    if (user_info.site_id.has_value()) {
        ostr << "Site: " << (*user_info.site_id + 0) << ", ";
    }
    ostr <<
        "ID: " << (user_info.user_id + 0) <<
        ", name: \"" << user_info.name << '"' <<
        ", full name: \"" << user_info.full_name << '"' <<
        ", type: " << "0x" << std::hex << (int)user_info.type <<
        ", random rank: " << (user_info.random_rank + 0) <<
        ", status: " << "0x" << std::hex << (int)user_info.get_status();
    return ostr;
}

SiteInfo::SiteInfo()
    : users{ 0 }
{}

SiteInfo::~SiteInfo() = default;

RemoteSites::RemoteSites(
    const DanglingBaseClassRef<Users>& local_users,
    const std::optional<RemoteParams>& remote_params)
    : local_users_{ local_users }
    , remote_params_{ remote_params }
    , remote_sites_{ "Site", [](RemoteSiteId site_id){ return std::to_string(site_id); } }
    , users_total_{ 0 }
    , users_with_loaded_level_{ 0 }
{}

RemoteSites::~RemoteSites() = default;

std::optional<RemoteSiteId> RemoteSites::get_local_site_id() const {
    if (remote_params_.has_value()) {
        return remote_params_->site_id;
    } else {
        return std::nullopt;
    }
}

NUserCountType RemoteSites::get_local_user_count() const {
    assert_local_users_consistents();
    return local_users_->get_user_count();
}

NUserCountType RemoteSites::get_user_count(RemoteSiteId site_id) const {
    std::shared_lock lock{ mutex_ };
    if (!remote_params_.has_value() || (site_id == remote_params_->site_id)) {
        return get_local_user_count();
    } else {
        return integral_cast<NUserCountType>(remote_sites_.get(site_id).users.size());
    }
}

NUserCountType RemoteSites::get_total_user_count(UserTypes user_types) const {
    NUserCountType result = 0;
    for_each_site_user(
        [&](const UserInfo& user){
            ++result;
            return true;
        }, user_types);
    return result;
}

void RemoteSites::set_local_user_count(NUserCountType user_count) {
    std::scoped_lock lock{ mutex_ };
    local_site_.users.clear_and_reserve(user_count);
    local_users_->set_user_count(0);
    if (remote_params_.has_value()) {
        for (NUserCountType i = 0; i < user_count; ++i) {
            auto name = std::to_string(i);
            auto full_name = VariableAndHash<std::string>{std::to_string(remote_params_->site_id) + '_' + name};
            auto& user = local_site_.users.emplace_back(
                DanglingBaseClassRef<RemoteSites>{*this, CURRENT_SOURCE_LOCATION},
                remote_params_->site_id, i, name, *full_name, UserType::LOCAL);
            named_users_.emplace(
                std::move(full_name),
                DanglingBaseClassRef<UserInfo>{user, CURRENT_SOURCE_LOCATION},
                CURRENT_SOURCE_LOCATION);
            local_users_->set_user_count(i + 1);
        }
    } else {
        for (NUserCountType i = 0; i < user_count; ++i) {
            auto name = VariableAndHash<std::string>{std::to_string(i)};
            auto& user = local_site_.users.emplace_back(
                DanglingBaseClassRef<RemoteSites>{*this, CURRENT_SOURCE_LOCATION},
                std::nullopt, i, *name, *name, UserType::LOCAL);
            named_users_.emplace(
                std::move(name),
                DanglingBaseClassRef<UserInfo>{user, CURRENT_SOURCE_LOCATION},
                CURRENT_SOURCE_LOCATION);
            local_users_->set_user_count(i + 1);
        }
    }
}

void RemoteSites::set_user_count(RemoteSiteId site_id, NUserCountType user_count) {
    std::scoped_lock lock{ mutex_ };
    if (user_count > 100) {
        throw std::runtime_error("User-count per site cannot be greater than 100");
    }
    if (!remote_params_.has_value() || (site_id == remote_params_->site_id)) {
        assert_local_users_consistents();
        if (local_users_->get_user_count() != user_count) {
            throw std::runtime_error(
                "Attempt to remotely set the user count to " +
                std::to_string(user_count) +
                " when it is actually " +
                std::to_string(local_users_->get_user_count()));
        }
    } else {
        if (!remote_sites_.contains(site_id)) {
            if (remote_sites_.size() >= 256) {
                throw std::runtime_error("Number of sites cannot be greater than 256");
            }
        } else if (user_count == get_user_count(site_id)) {
            return;
        }
        remote_sites_.erase(site_id);
        auto& site = remote_sites_.add(site_id);
        site.users.clear_and_reserve(user_count);
        for (NUserCountType i = 0; i < user_count; ++i) {
            auto name = std::to_string(i);
            auto full_name = VariableAndHash<std::string>{std::to_string(site_id) + '_' + name};
            auto& user = site.users.emplace_back(
                DanglingBaseClassRef<RemoteSites>{*this, CURRENT_SOURCE_LOCATION},
                site_id, i, name, *full_name, UserType::REMOTE);
            named_users_.emplace(
                std::move(full_name),
                DanglingBaseClassRef<UserInfo>{user, CURRENT_SOURCE_LOCATION},
                CURRENT_SOURCE_LOCATION);
        }
    }
}

bool RemoteSites::for_each_site_user(
    const std::function<bool(UserInfo& user)>& operation,
    UserTypes user_types)
{
    auto include_user = [&](const UserInfo& user){
        return (((user.get_status() == UserStatus::INITIAL) && any(user_types & UserTypes::INITIAL)) ||
                ((user.get_status() == UserStatus::LEVEL_LOADED) && any(user_types & UserTypes::LEVEL_LOADED)) ||
                ((user.get_status() == UserStatus::LEVEL_LOADING) && any(user_types & UserTypes::LEVEL_LOADING)));
    };
    std::shared_lock lock{ mutex_ };
    assert_local_users_consistents();
    if (remote_params_.has_value()) {
        if (any(user_types & UserTypes::REMOTE)) {
            for (auto& [site_id, site] : remote_sites_) {
                if (site_id != remote_params_->site_id) {
                    for (auto&& [user_id, user] : tenumerate<NUserCountType>(site.users)) {
                        if (include_user(user)) {
                            if (!operation(user)) {
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
    if (any(user_types & UserTypes::LOCAL)) {
        if (!local_users_->for_each_user([&](NUserCountType user_id){
            auto& user = local_site_.users.at(user_id);
            if (include_user(user)) {
                return operation(user);
            }
            return true;
        }))
        {
            return false;
        }
    }
    return true;
}

bool RemoteSites::for_each_site_user(
    const std::function<bool(const UserInfo& user)>& operation,
    UserTypes user_types) const
{
    return const_cast<RemoteSites*>(this)->for_each_site_user(
        [&](UserInfo& user){
            return operation(user);
        }, user_types);
}

DanglingBaseClassRef<UserInfo> RemoteSites::get_user(RemoteSiteId site_id, NUserCountType id) {
    auto& site = get_site_info(site_id);
    if (id >= site.users.size()) {
        throw std::runtime_error("User ID too large");
    }
    return DanglingBaseClassRef<UserInfo>{ site.users.at(id), CURRENT_SOURCE_LOCATION };
}

DanglingBaseClassRef<const UserInfo> RemoteSites::get_user(RemoteSiteId site_id, NUserCountType id) const {
    return const_cast<RemoteSites*>(this)->get_user(site_id, id);
}

DanglingBaseClassRef<UserInfo> RemoteSites::get_user(const VariableAndHash<std::string>& full_name) {
    auto it = named_users_.find(full_name);
    if (it == named_users_.end()) {
        throw std::runtime_error("Could not find user with name \"" + *full_name + '"');
    }
    return it->second.object();
}

DanglingBaseClassRef<const UserInfo> RemoteSites::get_user(
    const VariableAndHash<std::string>& full_name) const
{
    return const_cast<RemoteSites*>(this)->get_user(full_name);
}

bool RemoteSites::contains_user(const VariableAndHash<std::string>& full_name) const {
    return (named_users_.find(full_name) != named_users_.end());
}

DanglingBaseClassRef<const UserInfo> RemoteSites::get_local_user(NUserCountType id) const {
    return {local_site_.users.at(id), CURRENT_SOURCE_LOCATION};
}

void RemoteSites::print(std::ostream& ostr) const {
    for_each_site_user([&](const UserInfo& user){
        if (user.site_id.has_value()) {
            ostr << "Site: " << (*user.site_id + 0) << ", user: " << (user.user_id + 0) << ", " << user.full_name << '\n';
        } else {
            ostr << "User: " << (user.user_id + 0) << ", " << user.full_name << '\n';
        }
        return true;
    }, UserTypes::ALL);
}

DanglingBaseClassRef<const UserInfo> RemoteSites::get_user_by_rank(NUserCountType rank) const {
    auto result = try_get_user_by_rank(rank);
    if (result == nullptr) {
        throw std::runtime_error("Could not find user with rank " + std::to_string(rank));
    }
    return *result;
}

DanglingBaseClassPtr<const UserInfo> RemoteSites::try_get_user_by_rank(NUserCountType rank) const {
    DanglingBaseClassPtr<const UserInfo> result = nullptr;
    for_each_site_user([&](const UserInfo& user){
        if (user.random_rank == rank) {
            result = {user, CURRENT_SOURCE_LOCATION};
            return false;
        }
        return true;
    }, UserTypes::ALL);
    return result;
}

SiteInfo& RemoteSites::get_site_info(RemoteSiteId site_id) {
    if (!remote_params_.has_value() || (site_id == remote_params_->site_id)) {
        return local_site_;
    } else {
        return remote_sites_.get(site_id);
    }
}

void RemoteSites::assert_local_users_consistents() const {
    auto nlocal = local_users_->get_user_count();
    auto nremote = local_site_.users.size();
    if (nlocal != nremote) {
        throw std::runtime_error(
            "Number of local users (" + std::to_string(nlocal) +
            ") differs from the number of remote users (" + std::to_string(nremote) + ')');
    }
}

size_t RemoteSites::users_total() const {
    assert_true(users_total_ == named_users_.size());
    return named_users_.size();
}

NUserCountType RemoteSites::compute_random_user_ranks() {
    auto nusers = get_total_user_count(UserTypes::ALL);
    auto perm = arange<NUserCountType>(nusers);
    std::mt19937 rng_;
    rng_.seed(42);
    std::shuffle(perm.flat_begin(), perm.flat_end(), rng_);
    NUserCountType i = 0;
    for_each_site_user(
        [&](UserInfo& user){
            if (i >= perm.length()) {
                verbose_abort("RemoteSites::compute_random_user_rank: Number of users changed");
            }
            user.random_rank = perm(i);
            ++i;
            return true;
        }, UserTypes::ALL);
    return nusers;
}

NUserCountType RemoteSites::compute_free_user_rank() const {
    for (NUserCountType i = 0; i < 200; ++i) {
        bool is_free = for_each_site_user([&i](const UserInfo& user){
            return (i != user.random_rank);
        }, UserTypes::ALL);
        if (is_free) {
            return i;
        }
    }
    throw std::runtime_error("Could not compute rank for user (tried 0 - 200)");
}

void RemoteSites::set_user_status(UserTypes types, UserStatus status) {
    for_each_site_user(
        [&](UserInfo& user){
            user.set_status(status);
            return true;
        }, types);
}
