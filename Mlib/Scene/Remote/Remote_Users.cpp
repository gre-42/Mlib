#include "Remote_Users.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>

using namespace Mlib;

static_assert(sizeof(FixedArray<float, 3>) == 3 * 4);

RemoteUsers::RemoteUsers(
    ObjectPool& object_pool,
    IoVerbosity verbosity,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    RemoteSiteId site_id)
    : physics_scene_{ physics_scene }
    , verbosity_{ verbosity }
    , site_id_{ site_id }
    , physics_scene_on_destroy_{ physics_scene->on_destroy, CURRENT_SOURCE_LOCATION }
{
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Create RemoteUsers";
    }
    physics_scene_on_destroy_.add([&o=object_pool, this](){ o.remove(this); }, CURRENT_SOURCE_LOCATION);
}

RemoteUsers::~RemoteUsers() {
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Destroy RemoteUsers";
    }
    on_destroy.clear();
}

std::unique_ptr<RemoteUsers> RemoteUsers::try_create_from_stream(
    ObjectPool& object_pool,
    PhysicsScene& physics_scene,
    std::istream& istr,
    IoVerbosity verbosity,
    RemoteSiteId site_id)
{
    auto res = std::make_unique<RemoteUsers>(
        object_pool,
        verbosity,
        DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION},
        site_id);
    res->read_data(istr);
    return res;
}

void RemoteUsers::read(std::istream& istr) {
    auto type = read_binary<RemoteSceneObjectType>(istr, "scene object type", verbosity_);
    if (type != RemoteSceneObjectType::REMOTE_USERS) {
        THROW_OR_ABORT("RemoteUsers::read: Unexpected scene object type");
    }
    read_data(istr);
}

void RemoteUsers::read_data(std::istream& istr) {
    auto reader = BinaryReader(istr, verbosity_);
    auto user_count = reader.read_binary<uint32_t>("user count");
    physics_scene_->remote_sites_->set_user_count(site_id_, user_count);
    
    {
        auto args = physics_scene_->macro_line_executor_.writable_json_macro_arguments();
        for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
            {
                auto key = "selected_vehicle_id_" + std::to_string(site_id_) + '_' + std::to_string(user_id);
                args->set(key, reader.read_string("selected vehicle ID"));
            }
            {
                auto key = "selected_vehicle_color_" + std::to_string(site_id_) + '_' + std::to_string(user_id);
                args->set(key, reader.read_binary<EFixedArray<float, 3>>("selected vehicle color"));
            }
            {
                auto account = nlohmann::json::object();
                account["name"] = reader.read_string("account name");
                auto key = "account_" + std::to_string(site_id_) + '_' + std::to_string(user_id);
                args->set(key, account);
            }
        }
        args.unlock_and_notify();
    }
}

void RemoteUsers::write(std::ostream& ostr, ObjectCompression compression) {
    auto user_count = physics_scene_->remote_sites_->get_user_count(site_id_);
    
    auto writer = BinaryWriter{ostr};
    writer.write_binary(RemoteSceneObjectType::REMOTE_USERS, "remote users");
    writer.write_binary(user_count, "user count");
    {
        auto args = physics_scene_->macro_line_executor_.json_macro_arguments();
        for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
            // try {
            {
                auto key = "selected_vehicle_id_" + std::to_string(site_id_) + '_' + std::to_string(user_id);
                writer.write_string(args->at<std::string>(key), "selected vehicle ID");
            }
            {
                auto key = "selected_vehicle_color_" + std::to_string(site_id_) + '_' + std::to_string(user_id);
                writer.write_binary(args->at<EFixedArray<float, 3>>(key), "selected vehicle color");
            }
            {
                auto key = "account_" + std::to_string(site_id_) + '_' + std::to_string(user_id);
                auto account_name = args->resolve_t<std::string>(key, "name");
                writer.write_string(account_name, "account name");
            }
            // } catch (const std::runtime_error& e) {
            //     lerr() << "Could not write variable\n" << (const nlohmann::json&)args;
            //     throw;
            // }
        }
    }
}
