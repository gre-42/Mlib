#include "Remote_Users.hpp"
#include <Mlib/Io/Binary.hpp>
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
}

void RemoteUsers::write(std::ostream& ostr, ObjectCompression compression) {
    write_binary(ostr, RemoteSceneObjectType::REMOTE_USERS, "remote users");
    write_binary(ostr, physics_scene_->remote_sites_->get_user_count(site_id_), "user count");
}
