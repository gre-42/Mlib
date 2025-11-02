#include "Remote_Sites.hpp"
#include <Mlib/Players/Containers/Users.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

RemoteSites::RemoteSites(
    const DanglingBaseClassRef<Users>& local_users,
    const std::optional<RemoteParams>& remote_params)
    : local_users_{ local_users }
    , remote_params_{ remote_params }
    , sites_{ "Site", [](RemoteSiteId site_id){ return std::to_string(site_id); } }
{}

RemoteSites::~RemoteSites() = default;

uint32_t RemoteSites::get_user_count(RemoteSiteId site_id) const {
    std::shared_lock lock{ mutex_ };
    if (!remote_params_.has_value() || (site_id == remote_params_->site_id)) {
        return local_users_->get_user_count();
    } else {
        return sites_.get(site_id);
    }
}

void RemoteSites::set_user_count(RemoteSiteId site_id, uint32_t user_count) {
    std::scoped_lock lock{ mutex_ };
    sites_[site_id] = user_count;
}

void RemoteSites::for_each_site_user(const std::function<void(RemoteSiteId, uint32_t)>& operation) const {
    std::shared_lock lock{ mutex_ };
    if (remote_params_.has_value()) {
        local_users_->for_each_user([&](uint32_t user_id){
            operation(remote_params_->site_id, user_id);
        });
        for (const auto [site, user_count] : sites_) {
            if (site != remote_params_->site_id) {
                for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
                    operation(site, user_id);
                }
            }
        }
    } else {
        local_users_->for_each_user([&](uint32_t user_id){
            operation(0xA1B2C3D4, user_id);
        });
    }
}

void RemoteSites::print(std::ostream& ostr) const {
    for_each_site_user([&](RemoteSiteId site_id, uint32_t user_id){
        ostr << "Site: " << site_id << ", user: " << user_id << '\n';
    });
}
