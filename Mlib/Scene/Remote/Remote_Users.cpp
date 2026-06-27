#include "Remote_Users.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Remote/Incremental_Objects/Known_Fields.hpp>
#include <Mlib/Remote/Incremental_Objects/Object_Lifetime_Status.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Remote/Remote_Check.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Priority.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>

using namespace Mlib;

static_assert(sizeof(FixedArray<float, 3>) == 3 * 4);

enum class RemoteUsersTransmittedFields: TransmittedFieldsType {
    NONZERO = (TransmittedFieldsType)TransmittedFields::END,
    ALL = (TransmittedFieldsType)TransmittedFields::END << 1
};

inline TransmittedFields operator & (TransmittedFields a, RemoteUsersTransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a & (TransmittedFieldsType)b);
}

inline TransmittedFields operator | (TransmittedFields a, RemoteUsersTransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a | (TransmittedFieldsType)b);
}

inline TransmittedFields& operator |= (TransmittedFields& a, RemoteUsersTransmittedFields b) {
    (TransmittedFieldsType&)a |= (TransmittedFieldsType)b;
    return a;
}

RemoteUsers::RemoteUsers(
    IoVerbosity verbosity,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    const DanglingBaseClassRef<SceneLevelSelector>& local_scene_level_selector,
    RemoteSiteId site_id)
    : physics_scene_{ physics_scene }
    , local_scene_level_selector_{ local_scene_level_selector }
    , verbosity_{ verbosity }
    , site_id_{ site_id }
    , physics_scene_on_destroy_{ physics_scene->on_destroy.deflt, CURRENT_SOURCE_LOCATION }
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
    BinaryBitwiseWordsReader& reader,
    TransmittedFields transmitted_fields,
    ObjectLifetimeStatus lifetime_status,
    RemoteSiteId site_id,
    ProxyTasks proxy_tasks,
    TransmissionHistoryReader& transmission_history_reader,
    IoVerbosity verbosity)
{
    if (!any(transmitted_fields & RemoteUsersTransmittedFields::ALL)) {
        return nullptr;
    }
    auto res = global_object_pool.create_unique<RemoteUsers>(
        CURRENT_SOURCE_LOCATION,
        verbosity,
        DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION},
        DanglingBaseClassRef<SceneLevelSelector>{local_scene_level_selector, CURRENT_SOURCE_LOCATION},
        site_id);
    res->read_data(reader, transmitted_fields, proxy_tasks, transmission_history_reader);
    if (lifetime_status == ObjectLifetimeStatus::DELETED) {
        return nullptr;
    }
    return {res.release(), CURRENT_SOURCE_LOCATION};
}

std::string RemoteUsers::name() const {
    return "users";
}

int32_t RemoteUsers::priority() const {
    return RemoteSceneObjectPriority::REMOTE_USERS;
}

void RemoteUsers::read(
    BinaryBitwiseWordsReader& reader,
    RemoteSiteId sender_site_id,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    TransmittedFields transmitted_fields,
    ProxyObjectsCaches& proxy_objects_caches,
    const IncrementalVersionsRead& versions,
    TransmissionHistoryReader& transmission_history_reader)
{
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != RemoteSceneObjectType::REMOTE_USERS) {
        throw std::runtime_error("RemoteUsers::read: Unexpected scene object type");
    }
    read_data(reader, transmitted_fields, proxy_tasks, transmission_history_reader);
}

void RemoteUsers::read_data(
    BinaryBitwiseWordsReader& reader,
    TransmittedFields transmitted_fields,
    ProxyTasks proxy_tasks,
    TransmissionHistoryReader& transmission_history_reader)
{
    if (any(transmitted_fields & RemoteUsersTransmittedFields::ALL)) {
        auto user_count = reader.read_binary<NUserCountType>("#users");
        physics_scene_->remote_sites_->set_user_count(site_id_, user_count);
        if (user_count > 0) {
            if (!physics_scene_->remote_sites_->get_local_site_id().has_value()) {
                throw std::runtime_error("Local site ID not set");
            }
            auto local_site_id = *physics_scene_->remote_sites_->get_local_site_id();
            auto transmitted_count = reader.read_binary<NUserCountType>("#users transmitted");
            if (transmitted_count > 100) {
                throw std::runtime_error("Too many users transmitted");
            }
            std::vector<NUserCountType> users_transmitted(transmitted_count);
            // Read user IDs
            for (auto& user_id : users_transmitted) {
                user_id = reader.read_binary<NUserCountType>("user ID");
            }
            // Read global variables
            {
                auto args = physics_scene_->macro_line_executor_.writable_json_macro_arguments();
                for (auto user_id : users_transmitted) {
                    auto suffix = std::to_string(site_id_) + '_' + std::to_string(user_id);
                    {
                        auto key = "selected_vehicle_id_" + suffix;
                        args->set(key, reader.read_string<StringLengthType>("selected vehicle ID"));
                    }
                    {
                        auto key = "selected_vehicle_color_" + suffix;
                        args->set(key, reader.read_binary<EFixedArray<float, 3>>("selected vehicle color"));
                    }
                    {
                        auto account = nlohmann::json::object();
                        account["name"] = reader.read_string<StringLengthType>("account name");
                        auto key = "account_" + suffix;
                        args->set(key, account);
                    }
                }
                args.unlock_and_notify();
            }
            // Read status
            for (auto user_id : users_transmitted) {
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
                        throw std::runtime_error("Unknown user status");
                    }();
                    user->set_status(final_status);
                } else if (site_id_ != local_site_id) {
                    user->set_status(status);
                }
            }
        }
    }
    if (remote_end_check_enabled()) {
        auto end = reader.read_binary<RemoteSceneObjectType>("inverted remote users");
        if (end != ~RemoteSceneObjectType::REMOTE_USERS) {
            throw std::runtime_error("Invalid remote users end");
        }
    }
}

void RemoteUsers::write(
    BinaryBitwiseWordsWriter& writer,
    RemoteSiteId receiver_site_id,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    KnownFields known_fields,
    ProxyObjectsCaches& proxy_objects_caches,
    const IncrementalVersionsWrite& versions,
    TransmissionHistoryWriter& transmission_history_writer)
{
    auto transmitted_fields = TransmittedFields::NONE;
    transmitted_fields |= RemoteUsersTransmittedFields::NONZERO;
    if (known_fields == KnownFields::NONE) {
        transmitted_fields |= RemoteUsersTransmittedFields::ALL;
    }
    transmission_history_writer.write_remote_object_id(writer, remote_object_id, transmitted_fields);
    writer.write_binary(RemoteSceneObjectType::REMOTE_USERS, "remote users");
    if (any(transmitted_fields & RemoteUsersTransmittedFields::ALL)) {
        auto user_count = physics_scene_->remote_sites_->get_user_count(site_id_);
        writer.write_binary(user_count, "user count");
        if (user_count > 0) {
            auto user_id = integral_cast<NUserCountType>(transmission_history_writer.datagram_counter() % user_count);
            writer.write_binary((NUserCountType)1, "#users transmitted");
            writer.write_binary(user_id, "user ID");
            {
                auto args = physics_scene_->macro_line_executor_.json_macro_arguments();
                {
                    auto suffix = std::to_string(site_id_) + '_' + std::to_string(user_id);
                    // try {
                    {
                        auto key = "selected_vehicle_id_" + suffix;
                        writer.write_string<StringLengthType>(args->at<std::string>(key), "selected vehicle ID");
                    }
                    {
                        auto key = "selected_vehicle_color_" + suffix;
                        writer.write_binary(args->at<EFixedArray<float, 3>>(key), "selected vehicle color");
                    }
                    {
                        auto key = "account_" + suffix;
                        auto account_name = args->resolve_t<std::string>(key, "name");
                        writer.write_string<StringLengthType>(account_name, "account name");
                    }
                    // } catch (const std::runtime_error& e) {
                    //     lerr() << "Could not write variable\n" << (const nlohmann::json&)args;
                    //     throw;
                    // }
                }
            }
            {
                auto user = physics_scene_->remote_sites_->get_user(site_id_, user_id);
                writer.write_binary(user->get_status(), "user status");
            }
        }
    }
    if (remote_end_check_enabled()) {
        writer.write_binary(~RemoteSceneObjectType::REMOTE_USERS, "inverted remote users");
    }
}
