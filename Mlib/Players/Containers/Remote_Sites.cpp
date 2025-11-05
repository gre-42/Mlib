#include "Remote_Sites.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Players/Containers/Users.hpp>
#include <Mlib/Stats/Arange.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <random>

using namespace Mlib;

RemoteSites::RemoteSites(
    const DanglingBaseClassRef<Users>& local_users,
    const std::optional<RemoteParams>& remote_params)
    : local_users_{ local_users }
    , remote_params_{ remote_params }
    , remote_sites_{ "Site", [](RemoteSiteId site_id){ return std::to_string(site_id); } }
{}

RemoteSites::~RemoteSites() = default;

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

uint32_t RemoteSites::get_total_user_count(UserType user_type) const {
    uint32_t result = 0;
    for_each_site_user(
        [&](std::optional<RemoteSiteId> site_id, uint32_t user_id, const UserInfo& user){
            ++result;
        }, user_type);
    return result;
}

void RemoteSites::set_local_user_count(uint32_t user_count) {
    local_users_->set_user_count(user_count);
    local_site_.users.resize(user_count);
}

void RemoteSites::set_user_count(RemoteSiteId site_id, uint32_t user_count) {
    std::scoped_lock lock{ mutex_ };
    if (user_count > 256) {
        THROW_OR_ABORT("User-count per site cannot be greater than 256");
    }
    if (!remote_params_.has_value() || (site_id == remote_params_->site_id)) {
        assert_local_users_consistents();
        if (local_users_->get_user_count() != user_count) {
            if (local_users_->get_user_count() != 0) {
                THROW_OR_ABORT(
                    "Attempt to remotely set the user count to " +
                    std::to_string(user_count) +
                    "when it was previously already set to " +
                    std::to_string(local_users_->get_user_count()));
            }
            set_local_user_count(user_count);
        }
    } else {
        if (!remote_sites_.contains(site_id) && (remote_sites_.size() >= 256)) {
            THROW_OR_ABORT("Number of sites cannot be greater than 256");
        }
        remote_sites_[site_id].users.resize(user_count);
    }
}

void RemoteSites::for_each_site_user(
    const std::function<void(
        std::optional<RemoteSiteId>,
        uint32_t user_id,
        UserInfo& user)>& operation,
    UserType user_type)
{
    std::shared_lock lock{ mutex_ };
    assert_local_users_consistents();
    if (remote_params_.has_value()) {
        local_users_->for_each_user([&](uint32_t user_id){
            operation(remote_params_->site_id, user_id, local_site_.users.at(user_id));
        });
        if (user_type == UserType::ALL) {
            for (auto& [site_id, site] : remote_sites_) {
                if (site_id != remote_params_->site_id) {
                    for (auto&& [user_id, user] : tenumerate<uint32_t>(site.users)) {
                        operation(site_id, user_id, user);
                    }
                }
            }
        }
    } else {
        local_users_->for_each_user([&](uint32_t user_id){
            operation(std::nullopt, user_id, local_site_.users.at(user_id));
        });
    }
}

void RemoteSites::for_each_site_user(
    const std::function<void(
        std::optional<RemoteSiteId>,
        uint32_t user_id,
        const UserInfo& user)>& operation,
    UserType user_type) const
{
    const_cast<RemoteSites*>(this)->for_each_site_user(
        [&](std::optional<RemoteSiteId> site_id, uint32_t user_id, UserInfo& user){
            operation(site_id, user_id, user);
        }, user_type);
}

void RemoteSites::print(std::ostream& ostr) const {
    for_each_site_user([&](std::optional<RemoteSiteId> site_id, uint32_t user_id, const UserInfo& user){
        if (site_id.has_value()) {
            ostr << "Site: " << *site_id << ", user: " << user_id << '\n';
        } else {
            ostr << "User: " << user_id << '\n';
        }
    }, UserType::ALL);
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
    auto nusers = get_total_user_count(UserType::ALL);
    auto perm = arange<uint32_t>(nusers);
    std::mt19937 rng_;
    rng_.seed(42);
    std::shuffle(perm.flat_begin(), perm.flat_end(), rng_);
    uint32_t i = 0;
    for_each_site_user(
        [&](std::optional<RemoteSiteId> site_id, uint32_t user_id, UserInfo& user){
            if (i >= perm.length()) {
                verbose_abort("RemoteSites::compute_random_user_rank: Number of users changed");
            }
            user.random_rank = perm(i);
            ++i;
        }, UserType::ALL);
}
