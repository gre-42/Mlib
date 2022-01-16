#include "mlib.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Pod_Bot/bot.h>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/globals.hpp>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/pod_bot.hpp>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/types.hpp>

static std::map<const Mlib::RigidBodyIntegrator*, std::string> g_rbi_to_player_name;
extern std::map<int, edict_t*> indexent_;

void Mlib::pod_bot_destroy_player(const Player& player) {
    edict_t* edict = get_edict(player.name());
    auto cb = pod_bot_get_client_and_bot(edict);
    if (g_edict_to_player_name.erase(edict) != 1) {
        throw std::runtime_error("Could not erase edict");
    }
    if (g_player_name_to_edict.erase(player.name()) != 1) {
        throw std::runtime_error("Could not erase player");
    }
    // From: https://developer.valvesoftware.com/wiki/Entity_index
    // "Worldspawn is always entity 0, while indices 1 to <maxplayers> are reserved for players.""
    if (std::erase_if(indexent_, [&edict](const auto& it){ return it.second == edict; }) != 1) {
        throw std::runtime_error("Could not erase from indexent");
    }
    if (player.has_rigid_body()) {
        clear_player_rigid_body_integrator(player.rigid_body().rbi_);
    }
    delete cb.bot->pEdict;
    cb.bot->pEdict = nullptr;
}

void Mlib::pod_bot_set_players(Players& players, CollisionQuery& collision_query) {
    if (g_players != nullptr) {
        throw std::runtime_error("Players already set");
    }
    if (g_collision_query != nullptr) {
        throw std::runtime_error("Collision query already set");
    }
    g_players = &players;
    g_collision_query = &collision_query;
}

void Mlib::pod_bot_clear_players() {
    if (g_players == nullptr) {
        throw std::runtime_error("Players not set");
    }
    if (g_collision_query == nullptr) {
        throw std::runtime_error("Collision query not set");
    }
    g_players = nullptr;
    g_collision_query = nullptr;
}

int Mlib::pod_bot_team_id(const std::string& team_name) {
    // Copy & paste from podbot/bot.h
    // TEAM_CS_UNASSIGNED = 0,
    // TEAM_CS_TERRORIST = 1,
    // TEAM_CS_COUNTER = 2,
    // TEAM_CS_SPECTATOR = 3
    if (team_name == "terrorist") {
        return 1;
    } else if (team_name == "counter") {
        return 2;
    } else {
        throw std::runtime_error("Unknown team name, choose between \"terrorist\" and \"counter\"");
    }
    // static std::map<std::string, int> team_ids_;
    // auto it = team_ids_.insert({ team_name, team_ids_.size() });
    // return it.second;
}

void Mlib::set_player_rigid_body_integrator(
    const Mlib::RigidBodyIntegrator& rbi,
    const std::string& player_name)
{
    if (!g_rbi_to_player_name.insert({ &rbi, player_name }).second) {
        throw std::runtime_error("Could not set player rigid body integrator");
    }
}

void Mlib::clear_player_rigid_body_integrator(const Mlib::RigidBodyIntegrator& rbi)
{
    std::string player_name = get_player_name(rbi);
    edict_t* edict = get_edict(player_name);
    if (g_rbi_to_player_name.erase(&rbi) != 1) {
        throw std::runtime_error("Could not clear player rigid body integrator");
    }
    for (int player_index = 0; player_index < gpGlobals->maxClients; ++player_index)
    {
        bot_t& bot = bots[player_index];
        if (!FNullEnt(bot.pEdict)) {
            if (bot.pBotEnemy == edict) {
                bot.pBotEnemy = nullptr;
            }
            if (bot.pLastEnemy == edict) {
                bot.pLastEnemy = nullptr;
            }
            if (bot.pEdict->v.dmg_inflictor == edict) {
                bot.pEdict->v.dmg_inflictor = nullptr;
            }
        }
    }
}

std::string Mlib::get_player_name(const Mlib::RigidBodyIntegrator& rbi) {
    auto it = g_rbi_to_player_name.find(&rbi);
    if (it == g_rbi_to_player_name.end()) {
        throw std::runtime_error("Could not find player name for rigid body integrator");
    }
    return it->second;
}

edict_t* Mlib::get_edict(const std::string& player_name) {
    auto it = g_player_name_to_edict.find(player_name);
    if (it == g_player_name_to_edict.end()) {
        throw std::runtime_error("Could not find edict for player name");
    }
    return it->second;
}

Mlib::ClientAndBot Mlib::pod_bot_get_client_and_bot(edict_t* edict) {
    // From: https://developer.valvesoftware.com/wiki/Entity_index
    // "Worldspawn is always entity 0, while indices 1 to <maxplayers> are reserved for players.""
    int index = ENTINDEX(edict) - 1;
    if (index < 0 || index >= 32) {
        throw std::runtime_error("Client index out of bounds");
    }
    return Mlib::ClientAndBot{clients + index, bots + index};
}

void Mlib::pod_bot_initialize_edict(edict_t* edict) {
    const size_t PRIVATE_DATA_SIZE = 1000 * 1000;
    edict->pvPrivateData = malloc(PRIVATE_DATA_SIZE);
    std::fill((char*)edict->pvPrivateData, (char*)edict->pvPrivateData + PRIVATE_DATA_SIZE, 0);
}

Mlib::Player& Mlib::pod_bot_edict_to_player(const edict_t* edict) {
    auto pit = g_edict_to_player_name.find(const_cast<edict_t*>(edict));
    if (pit == g_edict_to_player_name.end()) {
        throw std::runtime_error("Could not find player for entity");
    }
    return g_players->get_player(pit->second);
}
