#include "Remote_Sites.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Players/Containers/Users.hpp>
#include <Mlib/Stats/Arange.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <random>

using namespace Mlib;

UserInfo::UserInfo(
    const std::optional<RemoteSiteId>& site_id,
    uint32_t user_id,
    std::string name,
    std::string full_name,
    UserType type)
    : site_id{ site_id }
    , user_id{ user_id }
    , name{ std::move(name) }
    , full_name{ std::move(full_name) }
    , type{ type }
    , random_rank{ 0 }
{}

UserInfo::~UserInfo() {
    on_destroy.clear();
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
{}

RemoteSites::~RemoteSites() = default;

std::optional<RemoteSiteId> RemoteSites::get_local_site_id() const {
    if (remote_params_.has_value()) {
        return remote_params_->site_id;
    } else {
        return std::nullopt;
    }
}

uint32_t RemoteSites::get_local_user_count() const {
    assert_local_users_consistents();
    return local_users_->get_user_count();
}

uint32_t RemoteSites::get_user_count(RemoteSiteId site_id) const {
    std::shared_lock lock{ mutex_ };
    if (!remote_params_.has_value() || (site_id == remote_params_->site_id)) {
        return get_local_user_count();
    } else {
        return integral_cast<uint32_t>(remote_sites_.get(site_id).users.size());
    }
}

uint32_t RemoteSites::get_total_user_count(UserTypes user_types) const {
    uint32_t result = 0;
    for_each_site_user(
        [&](const UserInfo& user){
            ++result;
            return true;
        }, user_types);
    return result;
}

void RemoteSites::set_local_user_count(uint32_t user_count) {
    std::scoped_lock lock{ mutex_ };
    local_users_->set_user_count(user_count);
    local_site_.users.clear_and_reserve(user_count);
    if (remote_params_.has_value()) {
        for (uint32_t i = 0; i < user_count; ++i) {
            auto name = std::to_string(i);
            auto full_name = VariableAndHash<std::string>{std::to_string(remote_params_->site_id) + '_' + name};
            auto& user = local_site_.users.emplace_back(remote_params_->site_id, i, name, *full_name, UserType::LOCAL);
            named_users_.emplace(
                std::move(full_name),
                DanglingBaseClassRef<UserInfo>{user, CURRENT_SOURCE_LOCATION},
                CURRENT_SOURCE_LOCATION);
        }
    } else {
        for (uint32_t i = 0; i < user_count; ++i) {
            auto name = VariableAndHash<std::string>{std::to_string(i)};
            auto& user = local_site_.users.emplace_back(std::nullopt, i, *name, *name, UserType::LOCAL);
            named_users_.emplace(
                std::move(name),
                DanglingBaseClassRef<UserInfo>{user, CURRENT_SOURCE_LOCATION},
                CURRENT_SOURCE_LOCATION);
        }
    }
}

void RemoteSites::set_user_count(RemoteSiteId site_id, uint32_t user_count) {
    std::scoped_lock lock{ mutex_ };
    if (user_count > 256) {
        THROW_OR_ABORT("User-count per site cannot be greater than 256");
    }
    if (!remote_params_.has_value() || (site_id == remote_params_->site_id)) {
        assert_local_users_consistents();
        if (local_users_->get_user_count() != user_count) {
            THROW_OR_ABORT(
                "Attempt to remotely set the user count to " +
                std::to_string(user_count) +
                " when it is actually " +
                std::to_string(local_users_->get_user_count()));
        }
    } else {
        if (!remote_sites_.contains(site_id)) {
            if (remote_sites_.size() >= 256) {
                THROW_OR_ABORT("Number of sites cannot be greater than 256");
            }
        } else if (user_count == get_user_count(site_id)) {
            return;
        }
        remote_sites_.erase(site_id);
        auto& site = remote_sites_.add(site_id);
        site.users.clear_and_reserve(user_count);
        for (uint32_t i = 0; i < user_count; ++i) {
            auto name = std::to_string(i);
            auto full_name = VariableAndHash<std::string>{std::to_string(site_id) + '_' + name};
            auto& user = site.users.emplace_back(site_id, i, name, *full_name, UserType::REMOTE);
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
    std::shared_lock lock{ mutex_ };
    assert_local_users_consistents();
    if (remote_params_.has_value()) {
        if (user_types == UserTypes::ALL) {
            for (auto& [site_id, site] : remote_sites_) {
                if (site_id != remote_params_->site_id) {
                    for (auto&& [user_id, user] : tenumerate<uint32_t>(site.users)) {
                        if (!operation(user)) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return local_users_->for_each_user([&](uint32_t user_id){
        return operation(local_site_.users.at(user_id));
    });
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

DanglingBaseClassRef<const UserInfo> RemoteSites::get_user(
    const VariableAndHash<std::string>& full_name) const
{
    auto it = named_users_.find(full_name);
    if (it == named_users_.end()) {
        THROW_OR_ABORT("Could not find user with name \"" + *full_name + '"');
    }
    return it->second.object();
}

DanglingBaseClassRef<const UserInfo> RemoteSites::get_local_user(uint32_t id) const {
    return {local_site_.users.at(id), CURRENT_SOURCE_LOCATION};
}

void RemoteSites::print(std::ostream& ostr) const {
    for_each_site_user([&](const UserInfo& user){
        if (user.site_id.has_value()) {
            ostr << "Site: " << *user.site_id << ", user: " << user.user_id << '\n';
        } else {
            ostr << "User: " << user.user_id << '\n';
        }
        return true;
    }, UserTypes::ALL);
}

DanglingBaseClassRef<const UserInfo> RemoteSites::get_user_by_rank(uint32_t rank) const {
    DanglingBaseClassPtr<const UserInfo> result = nullptr;
    for_each_site_user([&](const UserInfo& user){
        if (user.random_rank == rank) {
            result = {user, CURRENT_SOURCE_LOCATION};
            return false;
        }
        return true;
    }, UserTypes::ALL);
    if (result == nullptr) {
        THROW_OR_ABORT("Could not find user with rank " + std::to_string(rank));
    }
    return *result;
}

void RemoteSites::assert_local_users_consistents() const {
    auto nlocal = local_users_->get_user_count();
    auto nremote = local_site_.users.size();
    if (nlocal != nremote) {
        THROW_OR_ABORT(
            "Number of local users (" + std::to_string(nlocal) +
            ") differs from the number of remote users (" + std::to_string(nremote) + ')');
    }
}

void RemoteSites::compute_random_user_ranks() {
    auto nusers = get_total_user_count(UserTypes::ALL);
    auto perm = arange<uint32_t>(nusers);
    std::mt19937 rng_;
    rng_.seed(42);
    std::shuffle(perm.flat_begin(), perm.flat_end(), rng_);
    uint32_t i = 0;
    for_each_site_user(
        [&](UserInfo& user){
            if (i >= perm.length()) {
                verbose_abort("RemoteSites::compute_random_user_rank: Number of users changed");
            }
            user.random_rank = perm(i);
            ++i;
            return true;
        }, UserTypes::ALL);
}
