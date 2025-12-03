#include "Remote_Users.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>

using namespace Mlib;

static_assert(sizeof(FixedArray<float, 3>) == 3 * 4);

RemoteUsers::RemoteUsers(
    IoVerbosity verbosity,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    const DanglingBaseClassRef<SceneLevelSelector>& local_scene_level_selector,
    RemoteSiteId site_id)
    : physics_scene_{ physics_scene }
    , local_scene_level_selector_{ local_scene_level_selector }
    , verbosity_{ verbosity }
    , site_id_{ site_id }
    , physics_scene_on_destroy_{ physics_scene->on_destroy, CURRENT_SOURCE_LOCATION }
{
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Create RemoteUsers";
    }
    physics_scene_on_destroy_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

RemoteUsers::~RemoteUsers() {
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Destroy RemoteUsers";
    }
    on_destroy.clear();
}

DanglingBaseClassPtr<RemoteUsers> RemoteUsers::try_create_from_stream(
    PhysicsScene& physics_scene,
    SceneLevelSelector& local_scene_level_selector,
    std::istream& istr,
    RemoteSiteId site_id,
    ProxyTasks proxy_tasks,
    TransmissionHistoryReader& transmission_history_reader,
    IoVerbosity verbosity)
{
    auto res = global_object_pool.create_unique<RemoteUsers>(
        CURRENT_SOURCE_LOCATION,
        verbosity,
        DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION},
        DanglingBaseClassRef<SceneLevelSelector>{local_scene_level_selector, CURRENT_SOURCE_LOCATION},
        site_id);
    res->read_data(istr, proxy_tasks, transmission_history_reader);
    return {res.release(), CURRENT_SOURCE_LOCATION};
}

void RemoteUsers::read(
    std::istream& istr,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    TransmittedFields transmitted_fields,
    TransmissionHistoryReader& transmission_history_reader)
{
    auto type = read_binary<RemoteSceneObjectType>(istr, "scene object type", verbosity_);
    if (type != RemoteSceneObjectType::REMOTE_USERS) {
        THROW_OR_ABORT("RemoteUsers::read: Unexpected scene object type");
    }
    read_data(istr, proxy_tasks, transmission_history_reader);
}

void RemoteUsers::read_data(
    std::istream& istr,
    ProxyTasks proxy_tasks,
    TransmissionHistoryReader& transmission_history_reader)
{
    auto reader = BinaryReader(istr, verbosity_);
    auto user_count = reader.read_binary<uint32_t>("user count");
    physics_scene_->remote_sites_->set_user_count(site_id_, user_count);
    if (!physics_scene_->remote_sites_->get_local_site_id().has_value()) {
        THROW_OR_ABORT("Local site ID not set");
    }
    auto local_site_id = *physics_scene_->remote_sites_->get_local_site_id();
    {
        auto args = physics_scene_->macro_line_executor_.writable_json_macro_arguments();
        for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
            auto suffix = std::to_string(site_id_) + '_' + std::to_string(user_id);
            {
                auto key = "selected_vehicle_id_" + suffix;
                args->set(key, reader.read_string("selected vehicle ID"));
            }
            {
                auto key = "selected_vehicle_color_" + suffix;
                args->set(key, reader.read_binary<EFixedArray<float, 3>>("selected vehicle color"));
            }
            {
                auto account = nlohmann::json::object();
                account["name"] = reader.read_string("account name");
                auto key = "account_" + suffix;
                args->set(key, account);
            }
        }
        args.unlock_and_notify();
    }
    for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
        auto user = physics_scene_->remote_sites_->get_user(site_id_, user_id);
        auto status = reader.read_binary<UserStatus>("user status");
        if (!any(proxy_tasks & ProxyTasks::RELOAD_SCENE)) {
            auto final_status = [&](){
                switch (status) {
                case UserStatus::INITIAL:
                case UserStatus::LEVEL_LOADING:
                    return status;
                case UserStatus::LEVEL_LOADED:
                    {
                        if (local_scene_level_selector_->reload_required(transmission_history_reader.home_scene_level)) {
                            return UserStatus::INITIAL;
                        }
                        return status;
                    }
                }
                THROW_OR_ABORT("Unknown user status");
            }();
            user->set_status(final_status);
        } else if (site_id_ != local_site_id) {
            user->set_status(status);
        }
    }
    auto end = reader.read_binary<uint32_t>("inverted remote users");
    if (end != ~(uint32_t)RemoteSceneObjectType::REMOTE_USERS) {
        THROW_OR_ABORT("Invalid remote users end");
    }
}

void RemoteUsers::write(
    std::ostream& ostr,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    KnownFields known_fields,
    TransmissionHistoryWriter& transmission_history_writer)
{
    transmission_history_writer.write_remote_object_id(ostr, remote_object_id, TransmittedFields::END);
    auto user_count = physics_scene_->remote_sites_->get_user_count(site_id_);
    
    auto writer = BinaryWriter{ostr};
    writer.write_binary(RemoteSceneObjectType::REMOTE_USERS, "remote users");
    writer.write_binary(user_count, "user count");
    {
        auto args = physics_scene_->macro_line_executor_.json_macro_arguments();
        for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
            auto suffix = std::to_string(site_id_) + '_' + std::to_string(user_id);
            // try {
            {
                auto key = "selected_vehicle_id_" + suffix;
                writer.write_string(args->at<std::string>(key), "selected vehicle ID");
            }
            {
                auto key = "selected_vehicle_color_" + suffix;
                writer.write_binary(args->at<EFixedArray<float, 3>>(key), "selected vehicle color");
            }
            {
                auto key = "account_" + suffix;
                auto account_name = args->resolve_t<std::string>(key, "name");
                writer.write_string(account_name, "account name");
            }
            // } catch (const std::runtime_error& e) {
            //     lerr() << "Could not write variable\n" << (const nlohmann::json&)args;
            //     throw;
            // }
        }
    }
    for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
        auto user = physics_scene_->remote_sites_->get_user(site_id_, user_id);
        writer.write_binary(user->get_status(), "user status");
    }
    writer.write_binary(~(uint32_t)RemoteSceneObjectType::REMOTE_USERS, "inverted remote users");
}
