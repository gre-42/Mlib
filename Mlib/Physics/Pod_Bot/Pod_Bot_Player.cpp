#include "Pod_Bot_Player.hpp"
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <pod_bot/bot_globals.h>
#include <pod_bot_mlib_compat/mlib.hpp>

using namespace Mlib;

PodBotPlayer::PodBotPlayer(const Player& player)
: player_{ player }
{
    int bot_skill = 101;
    int bot_personality = 5;
    int bot_team = pod_bot_team_id(player.team());
    int bot_class = 5;
    const char* bot_name = player.name().c_str();

    BotCreate(
        bot_skill,
        bot_personality,
        bot_team,
        bot_class,
        bot_name);
    
    edict_t* edict = get_edict(player.name());

    pod_bot_initialize_edict(edict);

    auto cb = pod_bot_get_client_and_bot(edict);

    int weapon = CS_WEAPON_M4A1;
    int ammo = 999;

    cb.client->iTeam = bot_team;
    cb.client->iFlags |= CLIENT_ALIVE;
    cb.client->iCurrentWeaponId = weapon;

    cb.bot->current_weapon.iId = weapon;
    cb.bot->m_rgAmmoInClip[weapon] = ammo;
    cb.bot->current_weapon.iClip = ammo;
    cb.bot->current_weapon.iAmmo1 = 0;
    cb.bot->current_weapon.iAmmo2 = 0;
    cb.bot->f_view_distance = 500.f * s_o2q;
    cb.bot->f_maxview_distance = 500.f * s_o2q;

    edict->v.weapons = (1 << weapon);
}

PodBotPlayer::~PodBotPlayer() {
    edict_t* edict = get_edict(player_.name());
    auto cb = pod_bot_get_client_and_bot(edict);
    delete cb.bot->pEdict;
    cb.bot->pEdict = nullptr;

    pod_bot_destroy_player(player_);
}

void PodBotPlayer::set_rigid_body_integrator() {
    if (!player_.has_rigid_body()) {
        throw std::runtime_error("PodBotPlayer::set_rigid_body_integrator despite no rigid body set");
    }
    set_player_rigid_body_integrator(player_.rigid_body().rbi_, player_.name());
}

void PodBotPlayer::clear_rigid_body_integrator() {
    // Not working during destruction of the player's root node,
    // when the node is already deregistered.
    // if (!player_.has_rigid_body()) {
    if (player_.rb_ == nullptr) {
        throw std::runtime_error("PodBotPlayer::clear_rigid_body_integrator despite no rigid body set");
    }
    clear_player_rigid_body_integrator(player_.rb_->rbi_, player_.name());
}

void PodBotPlayer::update_health() {
    edict_t* edict = get_edict(player_.name());
    if (player_.rb_ == nullptr) {
        edict->v.health = 0;
    } else {
        edict->v.health = player_.rb_->damageable_->health();
    }
}
