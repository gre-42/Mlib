#include "Remote_Player.hpp"
#include <Mlib/Json/Base.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Misc/When_To_Equip.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Advance_Times/Player_Site_Privileges.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Seat.hpp>
#include <Mlib/Players/User_Account/User_Account.hpp>
#include <Mlib/Remote/Incremental_Objects/Known_Fields.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Remote/Remote_Check.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Create_Player.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Player_Args.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Set_Externals_Creator.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Events/Remote_Select_Next_Vehicle_History.hpp>
#include <Mlib/Scene/Remote/Remote_Events/Remote_Shot_History.hpp>
#include <Mlib/Scene/Remote/Remote_Privileges.hpp>
#include <Mlib/Scene/Remote/Remote_Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Priority.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>

using namespace Mlib;

enum class PlayerTransmittedFields: TransmittedFieldsType {
    NONZERO = (TransmittedFieldsType)TransmittedFields::END,
    SKILLS = (TransmittedFieldsType)TransmittedFields::END << 1,
};

inline TransmittedFields operator & (TransmittedFields a, PlayerTransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a & (TransmittedFieldsType)b);
}

inline TransmittedFields operator | (TransmittedFields a, PlayerTransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a | (TransmittedFieldsType)b);
}

inline TransmittedFields& operator |= (TransmittedFields& a, PlayerTransmittedFields b) {
    (TransmittedFieldsType&)a |= (TransmittedFieldsType)b;
    return a;
}

RemotePlayer::RemotePlayer(
    IoVerbosity verbosity,
    const DanglingBaseClassRef<Player>& player,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene)
    : player_{ player }
    , physics_scene_{ physics_scene }
    , vehicle_{ nullptr }
    , verbosity_{ verbosity }
    , player_on_destroy_{ player->on_destroy.deflt, CURRENT_SOURCE_LOCATION }
    , vehicle_on_destroy_{ nullptr, CURRENT_SOURCE_LOCATION }
{
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Create RemotePlayer";
    }
    player_on_destroy_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

RemotePlayer::~RemotePlayer() {
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Destroy RemotePlayer";
    }
    on_destroy.clear();
}

DanglingBaseClassPtr<RemotePlayer> RemotePlayer::try_create_from_stream(
    PhysicsScene& physics_scene,
    BinaryBitwiseWordsReader& reader,
    TransmittedFields transmitted_fields,
    TransmissionHistoryReader& transmission_history_reader,
    IoVerbosity verbosity)
{
    if (any(transmitted_fields & ~(
        TransmittedFields::SITE_ID |
        PlayerTransmittedFields::NONZERO |
        PlayerTransmittedFields::SKILLS)))
    {
        throw std::runtime_error("RemotePlayer::try_create_from_stream: Unknown transmitted fields");
    }
    bool has_scene_vehicle;
    auto args = nlohmann::json::object();
    std::optional<VariableAndHash<std::string>> name;
    std::optional<VariableAndHash<std::string>> full_user_name;
    if (any(transmitted_fields & PlayerTransmittedFields::SKILLS)) {
        name.emplace(reader.read_string<StringLengthType>("player ID"));
        args[PlayerArgs::name] = *name;
        args[PlayerArgs::team] = reader.read_string<StringLengthType>(PlayerArgs::team);
        full_user_name.emplace(reader.read_string<StringLengthType>(PlayerArgs::full_user_name));
        if (!(*full_user_name)->empty()) {
            args[PlayerArgs::full_user_name] = **full_user_name;
        }
        if (auto user_account_key = reader.read_string<StringLengthType>(PlayerArgs::user_account_key); !user_account_key.empty()) {
            args[PlayerArgs::user_account_key] = std::move(user_account_key);
        }
        args[PlayerArgs::game_mode] = game_mode_to_string(reader.read_bits<GameMode>(GAME_MODE_BITS, PlayerArgs::game_mode));
        args[PlayerArgs::player_role] = player_role_to_string(reader.read_bits<PlayerRole>(PLAYER_ROLE_BITS, PlayerArgs::player_role));
        args[PlayerArgs::unstuck_mode] = unstuck_mode_to_string(reader.read_bits<UnstuckMode>(UNSTUCK_MODE_BITS, PlayerArgs::unstuck_mode));
        args[PlayerArgs::driving_direction] = driving_direction_to_string(reader.read_bits<DrivingDirection>(DRIVING_DIRECTION_BITS, PlayerArgs::driving_direction));
        has_scene_vehicle = reader.read_bool_bit("has_scene_vehicle");
        args[PlayerArgs::behavior] = reader.read_string<StringLengthType>("behavior");
        reader.deserialize<Skills>("AI skills");
        reader.deserialize<Skills>("user skills");
    } else {
        has_scene_vehicle = reader.read_bool_bit("has_scene_vehicle");
    }
    if (has_scene_vehicle) {
        reader.deserialize<RemoteObjectId>("vehicle_object_id");
        reader.read_bits<VehicleSeat>(VEHICLE_SEAT_NBITS, "seat");
        reader.read_bits<ExternalsMode>(EXTERNALS_MODE_BITS, "externals mode");
        auto has_weapon_cycle = reader.read_bool_bit("has_weapon_cycle");
        auto has_gun_yaw = reader.read_bool_bit("has_gun_yaw");
        auto has_gun_pitch = reader.read_bool_bit("has_gun_pitch");
        if (has_weapon_cycle) {
            reader.read_string<StringLengthType>("weapon");
            read_shot_history(reader, transmission_history_reader);
        }
        if (has_gun_yaw) {
            reader.deserialize<CompressedSceneR16>("gun yaw");
        }
        if (has_gun_pitch) {
            reader.deserialize<CompressedSceneR16>("gun pitch");
        }
    }
    read_select_next_vehicle_history(reader, transmission_history_reader);
    if (remote_end_check_enabled()) {
        auto end = reader.read_binary<RemoteSceneObjectType>("inverted scene object type");
        if (end != ~RemoteSceneObjectType::PLAYER) {
            throw std::runtime_error("Invalid player message end (0). Player ID: \"" + *name.value_or(VariableAndHash<std::string>{"<not transmitted>"}) + '"');
        }
    }
    if (physics_scene.remote_scene_ == nullptr) {
        throw std::runtime_error("RemotePlayer: Remote scene is null");
    }
    if (!any(transmitted_fields & PlayerTransmittedFields::SKILLS)) {
        return nullptr;
    }
    if (full_user_name.has_value() &&
        !(*full_user_name)->empty() &&
        !physics_scene.remote_sites_->contains_user(*full_user_name))
    {
        linfo() << "Not creating player for user \"" << **full_user_name << '"';
        return nullptr;
    }
    CreatePlayer{physics_scene, physics_scene.macro_line_executor_}.execute(JsonView{args}, PlayerCreator::REMOTE);
    return {
        global_object_pool.create<RemotePlayer>(
            CURRENT_SOURCE_LOCATION,
            verbosity,
            physics_scene.players_.get_player(name.value(), CURRENT_SOURCE_LOCATION),
            DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION}),
        CURRENT_SOURCE_LOCATION};
}

std::string RemotePlayer::name() const {
    return *player_->id();
}

int32_t RemotePlayer::priority() const {
    return RemoteSceneObjectPriority::PLAYER;
}

void RemotePlayer::read(
    BinaryBitwiseWordsReader& reader,
    RemoteSiteId sender_site_id,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    TransmittedFields transmitted_fields,
    ProxyObjectsCaches& proxy_objects_caches,
    TransmissionHistoryReader& transmission_history_reader)
{
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != RemoteSceneObjectType::PLAYER) {
        throw std::runtime_error("RemotePlayer::read: Unexpected scene object type");
    }
    if (any(transmitted_fields & ~(
        TransmittedFields::SITE_ID |
        PlayerTransmittedFields::NONZERO |
        PlayerTransmittedFields::SKILLS)))
    {
        throw std::runtime_error("RemotePlayer::read: Unknown transmitted fields");
    }
    auto owner_site_id = [&](){
        auto user = player_->user_info();
        if (user == nullptr) {
            return remote_object_id.site_id;
        } else {
            if (!user->site_id.has_value()) {
                throw std::runtime_error("RemotePlayer::read: User site ID not set");
            }
            return *user->site_id;
        }
    }();
    auto privileges = RemotePrivileges{
        physics_scene_->remote_scene_->local_site_id(),
        sender_site_id,
        owner_site_id,
        remote_object_id.site_id};
    auto pp = privileges.position(PositionFlags::NONE);
    bool has_scene_vehicle;
    if (any(transmitted_fields & PlayerTransmittedFields::SKILLS)) {
        auto player_id = reader.read_string<StringLengthType>("player ID");
        reader.read_string<StringLengthType>("team");
        reader.read_string<StringLengthType>("full_user_name");
        reader.read_string<StringLengthType>("user_account_key");
        reader.read_bits<GameMode>(GAME_MODE_BITS, "game_mode");
        reader.read_bits<PlayerRole>(PLAYER_ROLE_BITS, "player_role");
        reader.read_bits<UnstuckMode>(UNSTUCK_MODE_BITS, "unstuck_mode");
        reader.read_bits<DrivingDirection>(DRIVING_DIRECTION_BITS, "driving_direction");
        has_scene_vehicle = reader.read_bool_bit("has_scene_vehicle");
        reader.read_string<StringLengthType>("behavior");
        player_->set_skills(ControlSource::AI, reader.deserialize<Skills>("AI skills"));
        player_->set_skills(ControlSource::USER, reader.deserialize<Skills>("user skills"));
    } else {
        has_scene_vehicle = reader.read_bool_bit("has_scene_vehicle");
    }
    if (has_scene_vehicle) {
        auto vehicle_object_id = reader.deserialize<RemoteObjectId>("vehicle_object_id");
        auto seat = vehicle_seat_to_string(reader.read_bits<VehicleSeat>(VEHICLE_SEAT_NBITS, "seat"));
        auto externals_mode = reader.read_bits<ExternalsMode>(EXTERNALS_MODE_BITS, "externals mode");
        if (!privileges.is_manager_local) {
            if (player_->has_scene_vehicle()) {
                auto rb = player_->rigid_body();
                if (!rb->remote_object_id_.has_value()) {
                    throw std::runtime_error("remote vehicle object ID not set");
                }
                if (*rb->remote_object_id_ != vehicle_object_id) {
                    reset_node();
                }
            }
            if (physics_scene_->remote_scene_ == nullptr) {
                throw std::runtime_error("RemotePlayer: Remote scene is null");
            }
            if (vehicle_ == nullptr) {
                auto ro = physics_scene_->remote_scene_->try_get(vehicle_object_id);
                if (ro != nullptr) {
                    auto rbv = dynamic_cast<RemoteRigidBodyVehicle*>(ro.get());
                    if (rbv == nullptr) {
                        throw std::runtime_error("Remote object is not a RemoteRigidBodyVehicle");
                    }
                    auto rb = rbv->rb();
                    if (rb->scene_node_ == nullptr) {
                        throw std::runtime_error("Rigid body has no scene node");
                    }
                    if (!rb->is_deactivated_avatar() && rb->drivers_.seat_is_free(seat)) {
                        vehicle_ = {
                            global_object_pool.create<SceneVehicle>(
                                CURRENT_SOURCE_LOCATION,
                                rb->node_name_,
                                *rb->scene_node_,
                                rb.set_loc(CURRENT_SOURCE_LOCATION)),
                            CURRENT_SOURCE_LOCATION};
                        vehicle_on_destroy_.set(vehicle_->on_destroy.deflt, CURRENT_SOURCE_LOCATION);
                        vehicle_on_destroy_.add([this](){ vehicle_ = nullptr; }, CURRENT_SOURCE_LOCATION);
                        player_->set_scene_vehicle(*vehicle_.get(), seat);
                        {
                            auto let = nlohmann::json::object({
                                {"asset_id", rb->asset_id_},
                                {"suffix", rbv->node_suffix()},
                                {"if_damageable", (rb->damageable_ != nullptr)}
                            });
                            SetExternalsCreator{
                                physics_scene_.get(),
                                physics_scene_->macro_line_executor_.inserted_block_arguments(std::move(let))
                            }.execute_safe(
                                *vehicle_.get(),
                                rb->asset_id_);
                        }
                        player_->create_vehicle_externals(externals_mode);
                        player_->create_vehicle_internals({ seat });
                        player_->create_gun_externals();
                    }
                }
            }
        }
        auto has_weapon_cycle = reader.read_bool_bit("has_weapon_cycle");
        auto has_gun_yaw = reader.read_bool_bit("has_gun_yaw");
        auto has_gun_pitch = reader.read_bool_bit("has_gun_pitch");
        if (has_weapon_cycle) {
            auto weapon = reader.read_string<StringLengthType>("weapon");
            auto shot_history = read_shot_history(reader, transmission_history_reader);
            if (pp.update_position && player_->has_scene_vehicle()) {
                auto rb = player_->rigid_body();
                if (!rb->remote_object_id_.has_value()) {
                    throw std::runtime_error("remote vehicle object ID not set");
                }
                if (*rb->remote_object_id_ == vehicle_object_id) {
                    player_->shot_history = std::move(shot_history);
                    if (has_weapon_cycle != player_->has_weapon_cycle()) {
                        throw std::runtime_error("Inconsistent \"has_weapon_cycle\"");
                    }
                    player_->set_desired_weapon(weapon, WhenToEquip::EQUIP_INSTANTLY);
                }
            }
        }
        if (has_gun_yaw) {
            auto gun_yaw = reader.read_binary<CompressedSceneR16>("gun yaw");
            if (pp.update_position && player_->has_gun_yaw()) {
                player_->set_gun_yaw((SceneDir)gun_yaw);
            }
        }
        if (has_gun_pitch) {
            auto gun_pitch = reader.read_binary<CompressedSceneR16>("gun pitch");
            if (pp.update_position && player_->has_gun_pitch()) {
                player_->set_gun_pitch((SceneDir)gun_pitch);
            }
        }
    } else if (!privileges.is_manager_local) {
        reset_node();
    }
    {
        auto select_next_vehicle_history = read_select_next_vehicle_history(reader, transmission_history_reader);
        if (pp.update_position) {
            player_->select_next_vehicle_history = std::move(select_next_vehicle_history);
        }
    }
    if (remote_end_check_enabled()) {
        auto end = reader.read_binary<RemoteSceneObjectType>("inverted scene object type");
        if (end != ~RemoteSceneObjectType::PLAYER) {
            throw std::runtime_error("Invalid player message end (1). Player-ID: \"" + *player_->id() + '"');
        }
    }
}

void RemotePlayer::reset_node() {
    player_->reset_node();
    vehicle_ = nullptr;
    vehicle_on_destroy_.clear();
}

void RemotePlayer::write(
    BinaryBitwiseWordsWriter& writer,
    RemoteSiteId receiver_site_id,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    KnownFields known_fields,
    ProxyObjectsCaches& proxy_objects_caches,
    TransmissionHistoryWriter& transmission_history_writer)
{
    auto transmitted_fields = TransmittedFields::NONE;
    transmitted_fields |= PlayerTransmittedFields::NONZERO;
    if (known_fields == KnownFields::NONE) {
        transmitted_fields |= PlayerTransmittedFields::SKILLS;
    }
    transmission_history_writer.write_remote_object_id(writer, remote_object_id, transmitted_fields);
    writer.write_binary(RemoteSceneObjectType::PLAYER, "scene object type");
    auto has_scene_vehicle = player_->has_scene_vehicle();
    if (any(transmitted_fields & PlayerTransmittedFields::SKILLS)) {
        writer.write_string<StringLengthType>(*player_->id(), "player name");
        writer.write_string<StringLengthType>(player_->team_name(), "player team");
        if (auto u = player_->user_info(); u != nullptr) {
            writer.write_string<StringLengthType>(u->full_name, "player full username (0)");
        } else {
            writer.write_string<StringLengthType>("", "player full username (1)");
        }
        if (auto a = player_->user_account(); a != nullptr) {
            writer.write_string<StringLengthType>(a->key(), "player user account key (0)");
        } else {
            writer.write_string<StringLengthType>("", "player user account key (1)");
        }
        writer.write_bits(player_->game_mode(), GAME_MODE_BITS, "player game mode");
        writer.write_bits(player_->player_role(), PLAYER_ROLE_BITS, "player role");
        writer.write_bits(player_->unstuck_mode(), UNSTUCK_MODE_BITS, "player unstuck mode");
        writer.write_bits(player_->driving_direction(), DRIVING_DIRECTION_BITS, "player driving direction");
        writer.write_bool_bit(has_scene_vehicle, "has_scene_vehicle");
        writer.write_string<StringLengthType>(player_->behavior(), "player behavior");
        writer.serialize(player_->get_skills(ControlSource::AI), "AI skills");
        writer.serialize(player_->get_skills(ControlSource::USER), "user skills");
    } else {
        writer.write_bool_bit(has_scene_vehicle, "has_scene_vehicle");
    }
    if (has_scene_vehicle) {
        auto rb = player_->rigid_body();
        if (!rb->remote_object_id_.has_value()) {
            throw std::runtime_error("remote vehicle object ID not set");
        }
        writer.serialize(*rb->remote_object_id_, "remote object ID");
        writer.write_bits(vehicle_seat_from_string(player_->seat()), VEHICLE_SEAT_NBITS, "seat");
        writer.write_bits(player_->externals_mode(), EXTERNALS_MODE_BITS, "externals mode");
        bool has_weapon_cycle = player_->has_weapon_cycle();
        bool has_gun_yaw = player_->has_gun_yaw();
        bool has_gun_pitch = player_->has_gun_pitch();
        writer.write_bool_bit(has_weapon_cycle, "has_weapon_cycle");
        writer.write_bool_bit(has_gun_yaw, "has_gun_yaw");
        writer.write_bool_bit(has_gun_pitch, "has_gun_pitch");
        if (has_weapon_cycle) {
            writer.write_string<StringLengthType>(player_->weapon_cycle()->weapon_name(), "weapon");
            write_shot_history(player_->shot_history, writer, transmission_history_writer);
        }
        if (has_gun_yaw) {
            writer.write_binary((CompressedSceneR16)player_->get_gun_yaw(), "gun yaw");
        }
        if (has_gun_pitch) {
            writer.write_binary((CompressedSceneR16)player_->get_gun_pitch(), "gun pitch");
        }
    }
    write_select_next_vehicle_history(player_->select_next_vehicle_history, writer, transmission_history_writer);
    if (remote_end_check_enabled()) {
        writer.write_binary(~RemoteSceneObjectType::PLAYER, "inverted scene object type");
    }
    writer.flush_partial("flush player");
}
