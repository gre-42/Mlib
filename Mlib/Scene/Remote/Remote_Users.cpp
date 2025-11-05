#include "Remote_Users.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>

using namespace Mlib;

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
    auto user_count = read_binary<uint32_t>(istr, "user count", verbosity_);
    physics_scene_->remote_sites_->set_user_count(site_id_, user_count);
    
    {
        auto args = physics_scene_->globals_->writable_json_macro_arguments();
        for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
            auto selected_vehicle_key = "selected_vehicle_id_" + std::to_string(site_id_) + '_' + std::to_string(user_id);
            args->set(selected_vehicle_key, read_string(istr, "selected vehicle"));
        }
        args.unlock_and_notify();
    }
}

void RemoteUsers::write(std::ostream& ostr, ObjectCompression compression) {
    auto user_count = physics_scene_->remote_sites_->get_user_count(site_id_);
    
    write_binary(ostr, RemoteSceneObjectType::REMOTE_USERS, "remote users");
    write_binary(ostr, user_count, "user count");
    {
        auto args = physics_scene_->globals_->json_macro_arguments();
        for (uint32_t user_id = 0; user_id < user_count; ++user_id) {
            auto selected_vehicle_key = "selected_vehicle_id_" + std::to_string(site_id_) + '_' + std::to_string(user_id);
            // try {
            write_string(ostr, args->at<std::string>(selected_vehicle_key), "selected vehicle");
            // } catch (const std::runtime_error& e) {
            //     lerr() << "Could not write variable\n" << (const nlohmann::json&)args;
            //     throw;
            // }
        }
    }
}

std::string RemoteUsers::read_string(std::istream& istr, const char* msg) const {
    auto len = read_binary<uint32_t>(istr, msg, verbosity_);
    std::string result;
    Mlib::read_string(istr, len, msg, verbosity_);
    return result;
}

void RemoteUsers::write_string(std::ostream& ostr, const std::string& str, const char* msg) const {
    write_binary(ostr, integral_cast<uint32_t>(str.length()), msg);
    Mlib::write_iterable(ostr, str, msg);
}
