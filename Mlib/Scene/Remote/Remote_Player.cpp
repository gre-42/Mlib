#include "Remote_Player.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Players/User_Account/User_Account.hpp>
#include <Mlib/Remote/Object_Compression.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Players/Create_Player.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <nlohmann/json.hpp>

using namespace Mlib;

RemotePlayer::RemotePlayer(
    ObjectPool& object_pool,
    IoVerbosity verbosity,
    const DanglingBaseClassRef<Player>& player)
    : player_{ player }
    , verbosity_{ verbosity }
    , player_on_destroy_{ player->on_destroy, CURRENT_SOURCE_LOCATION }
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

std::unique_ptr<RemotePlayer> RemotePlayer::try_create_from_stream(
    ObjectPool& object_pool,
    PhysicsScene& physics_scene,
    std::istream& istr,
    IoVerbosity verbosity)
{
    BinaryReader reader{ istr, verbosity };
    auto compression = reader.read_binary<ObjectCompression>("object compression");
    if (compression == ObjectCompression::NONE) {
        // Continue
    } else if (compression == ObjectCompression::INCREMENTAL) {
        return nullptr;
    } else {
        THROW_OR_ABORT("RemotePlayer::try_create_from_stream: Unknown compression mode");
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
    physics_scene.remote_scene_->created_at_remote_site.players.add(name);
    CreatePlayer{physics_scene}.execute(JsonView{args}, physics_scene.macro_line_executor_);
    return std::make_unique<RemotePlayer>(
        object_pool,
        verbosity,
        physics_scene.players_.get_player(name, CURRENT_SOURCE_LOCATION));
}

void RemotePlayer::read(std::istream& istr) {
    BinaryReader reader{ istr, verbosity_ };
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != RemoteSceneObjectType::PLAYER) {
        THROW_OR_ABORT("RemotePlayer::read: Unexpected scene object type");
    }
    auto compression = reader.read_binary<ObjectCompression>("object compression");
    if ((compression != ObjectCompression::NONE) &&
        (compression != ObjectCompression::INCREMENTAL))
    {
        THROW_OR_ABORT("RemotePlayer::read: Unknown compression mode");
    }
    reader.read_string("player ID");
    reader.read_string("team");
    reader.read_string("full_user_name");
    reader.read_string("user_account_key");
    reader.read_binary<GameMode>("game_mode");
    reader.read_binary<PlayerRole>("player_role");
    reader.read_binary<UnstuckMode>("unstuck_mode");
    reader.read_string("behavior");
    reader.read_binary<DrivingDirection>("driving_direction");
}

void RemotePlayer::write(std::ostream& ostr, ObjectCompression compression) {
    auto writer = BinaryWriter{ostr};
    writer.write_binary(RemoteSceneObjectType::PLAYER, "scene object type");
    writer.write_binary(compression, "player object compression");
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
}
