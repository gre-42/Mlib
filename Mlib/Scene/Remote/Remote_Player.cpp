#include "Remote_Player.hpp"
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/User_Account/User_Account.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Create_Player.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Externals_Creator.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <nlohmann/json.hpp>

using namespace Mlib;

RemotePlayer::RemotePlayer(
    ObjectPool& object_pool,
    IoVerbosity verbosity,
    const DanglingBaseClassRef<Player>& player,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene)
    : player_{ player }
    , physics_scene_{ physics_scene }
    , vehicle_{ nullptr }
    , verbosity_{ verbosity }
    , player_on_destroy_{ player->on_destroy, CURRENT_SOURCE_LOCATION }
    , vehicle_on_destroy_{ nullptr, CURRENT_SOURCE_LOCATION }
{
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Create RemotePlayer";
    }
    player_on_destroy_.add([&o=object_pool, this](){ o.remove(this); }, CURRENT_SOURCE_LOCATION);
}

RemotePlayer::~RemotePlayer() {
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Destroy RemotePlayer";
    }
    on_destroy.clear();
}

DanglingBaseClassPtr<RemotePlayer> RemotePlayer::try_create_from_stream(
    ObjectPool& object_pool,
    PhysicsScene& physics_scene,
    std::istream& istr,
    TransmittedFields transmitted_fields,
    IoVerbosity verbosity)
{
    BinaryReader reader{ istr, verbosity };
    if (any(transmitted_fields & ~(TransmittedFields::SITE_ID | TransmittedFields::END))) {
        THROW_OR_ABORT("RemotePlayer::try_create_from_stream: Unknown transmitted fields");
    }
    auto args = nlohmann::json::object();
    auto name = VariableAndHash<std::string>{reader.read_string("player ID")};
    args["name"] = *name;
    args["team"] = reader.read_string("team");
    args["full_user_name"] = reader.read_string("full_user_name");
    args["user_account_key"] = reader.read_string("user_account_key");
    args["game_mode"] = game_mode_to_string(reader.read_binary<GameMode>("game_mode"));
    args["player_role"] = player_role_to_string(reader.read_binary<PlayerRole>("player_role"));
    args["unstuck_mode"] = unstuck_mode_to_string(reader.read_binary<UnstuckMode>("unstuck_mode"));
    args["behavior"] = reader.read_string("behavior");
    args["driving_direction"] = driving_direction_to_string(reader.read_binary<DrivingDirection>("driving_direction"));
    Skills{}.read(istr);
    Skills{}.read(istr);
    auto has_scene_vehicle = reader.read_binary<bool>("has_scene_vehicle");
    if (has_scene_vehicle) {
        reader.read_binary<RemoteObjectId>("vehicle_object_id");
        reader.read_string("seat");
        reader.read_binary<ExternalsMode>("externals mode");
    }
    auto end = reader.read_binary<uint32_t>("inverted scene object type");
    if (end != ~(uint32_t)RemoteSceneObjectType::PLAYER) {
        THROW_OR_ABORT("Invalid player message end (0). Player ID: \"" + *name + '"');
    }
    if (physics_scene.remote_scene_ == nullptr) {
        THROW_OR_ABORT("RemotePlayer: Remote scene is null");
    }
    physics_scene.remote_scene_->created_at_remote_site.players.add(name);
    CreatePlayer{physics_scene}.execute(JsonView{args}, physics_scene.macro_line_executor_);
    return {
        object_pool.create<RemotePlayer>(
            CURRENT_SOURCE_LOCATION,
            object_pool,
            verbosity,
            physics_scene.players_.get_player(name, CURRENT_SOURCE_LOCATION),
            DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION}),
        CURRENT_SOURCE_LOCATION};
}

void RemotePlayer::read(
    std::istream& istr,
    TransmittedFields transmitted_fields)
{
    BinaryReader reader{ istr, verbosity_ };
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != RemoteSceneObjectType::PLAYER) {
        THROW_OR_ABORT("RemotePlayer::read: Unexpected scene object type");
    }
    if (any(transmitted_fields & ~(TransmittedFields::SITE_ID | TransmittedFields::END))) {
        THROW_OR_ABORT("RemotePlayer::read: Unknown transmitted fields");
    }
    auto player_id = reader.read_string("player ID");
    reader.read_string("team");
    reader.read_string("full_user_name");
    reader.read_string("user_account_key");
    reader.read_binary<GameMode>("game_mode");
    reader.read_binary<PlayerRole>("player_role");
    reader.read_binary<UnstuckMode>("unstuck_mode");
    reader.read_string("behavior");
    reader.read_binary<DrivingDirection>("driving_direction");
    player_->set_skills(ControlSource::AI, Skills{}.read(istr));
    player_->set_skills(ControlSource::USER, Skills{}.read(istr));
    auto has_scene_vehicle = reader.read_binary<bool>("has_scene_vehicle");
    if (has_scene_vehicle) {
        auto vehicle_object_id = reader.read_binary<RemoteObjectId>("vehicle_object_id");
        auto seat = reader.read_string("seat");
        auto externals_mode = reader.read_binary<ExternalsMode>("externals mode");
        if (vehicle_ == nullptr) {
            if (physics_scene_->remote_scene_ == nullptr) {
                THROW_OR_ABORT("RemotePlayer: Remote scene is null");
            }
            auto ro = physics_scene_->remote_scene_->try_get(vehicle_object_id);
            if (ro != nullptr) {
                auto rbv = dynamic_cast<RemoteRigidBodyVehicle*>(ro.get());
                if (rbv == nullptr) {
                    THROW_OR_ABORT("Remote object is not a RemoteRigidBodyVehicle");
                }
                auto rb = rbv->rb();
                if (rb->scene_node_ == nullptr) {
                    THROW_OR_ABORT("Rigid body has no scene node");
                }
                vehicle_ = {
                    global_object_pool.create<SceneVehicle>(
                        CURRENT_SOURCE_LOCATION,
                        physics_scene_->delete_node_mutex_,
                        rb->node_name_,
                        *rb->scene_node_,
                        rb.set_loc(CURRENT_SOURCE_LOCATION)),
                    CURRENT_SOURCE_LOCATION};
                vehicle_on_destroy_.set(vehicle_->on_destroy, CURRENT_SOURCE_LOCATION);
                vehicle_on_destroy_.add([this](){ vehicle_ = nullptr; }, CURRENT_SOURCE_LOCATION);
                auto user_info = player_->user_info();
                if (physics_scene_->remote_scene_ == nullptr) {
                    THROW_OR_ABORT("Remote scene is unexpectedly null");
                }
                if (user_info->site_id == physics_scene_->remote_scene_->local_site_id()) {
                    auto let = nlohmann::json::object({
                        {"local_user_id", user_info->user_id},
                        {"asset_id", rb->asset_id_},
                        {"suffix", rbv->node_suffix()}
                    });
                    SetExternalsCreator{ physics_scene_.get() }.execute_safe(
                        *vehicle_.get(),
                        rb->asset_id_,
                        physics_scene_->macro_line_executor_.inserted_block_arguments(std::move(let)));
                    player_->set_scene_vehicle(*vehicle_.get(), seat);
                    player_->create_vehicle_externals(externals_mode);
                    player_->create_vehicle_internals({ seat });
                }
            }
        }
    }
    auto end = reader.read_binary<uint32_t>("inverted scene object type");
    if (end != ~(uint32_t)RemoteSceneObjectType::PLAYER) {
        THROW_OR_ABORT("Invalid player message end (1). Player-ID: \"" + player_id + '"');
    }
}

void RemotePlayer::write(
    std::ostream& ostr,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    KnownFields known_fields,
    TransmissionHistoryWriter& transmission_history_writer)
{
    transmission_history_writer.write(ostr, remote_object_id, TransmittedFields::END);
    auto writer = BinaryWriter{ostr};
    writer.write_binary(RemoteSceneObjectType::PLAYER, "scene object type");
    writer.write_string(*player_->id(), "player name");
    writer.write_string(player_->team_name(), "player team");
    if (auto u = player_->user_info(); u != nullptr) {
        writer.write_string(u->full_name, "player full username (0)");
    } else {
        writer.write_string("", "player full username (1)");
    }
    if (auto a = player_->user_account(); a != nullptr) {
        writer.write_string(a->key(), "player user account key (0)");
    } else {
        writer.write_string("", "player user account key (1)");
    }
    writer.write_binary(player_->game_mode(), "player game mode");
    writer.write_binary(player_->player_role(), "player role");
    writer.write_binary(player_->unstuck_mode(), "player unstuck mode");
    writer.write_string(player_->behavior(), "player behavior");
    writer.write_binary(player_->driving_direction(), "player driving direction");
    player_->get_skills(ControlSource::AI).write(ostr);
    player_->get_skills(ControlSource::USER).write(ostr);
    if (player_->has_scene_vehicle()) {
        auto rb = player_->rigid_body();
        if (!rb->remote_object_id_.has_value()) {
            THROW_OR_ABORT("remote vehicle object ID not set");
        }
        writer.write_binary(true, "has_scene_vehicle (true)");
        writer.write_binary(*rb->remote_object_id_, "remote object ID");
        writer.write_string(player_->seat(), "seat");
        writer.write_binary(player_->externals_mode(), "externals mode");
    } else {
        writer.write_binary(false, "has_scene_vehicle (false)");
    }
    writer.write_binary(~(uint32_t)RemoteSceneObjectType::PLAYER, "inverted scene object type");
}
