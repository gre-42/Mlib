#pragma once
#include <pod_bot_mlib_compat/types.hpp>

struct client_t;
struct bot_t;

namespace Mlib {

class Player;
class Players;
class CollisionQuery;
class RigidBodyIntegrator;

struct ClientAndBot {
    client_t* client;
    bot_t* bot;
};

void pod_bot_set_players(Players& players, CollisionQuery& collision_query);

int pod_bot_team_id(const std::string& team_name);

Player& pod_bot_edict_to_player(const edict_t* edict);

void set_player_rigid_body_integrator(const RigidBodyIntegrator& rbi, const std::string& player_name);

std::string get_player_name(const RigidBodyIntegrator& rbi);

edict_t* get_edict(const std::string& player_name);

ClientAndBot pod_bot_get_client_and_bot(edict_t* edict);

void pod_bot_initialize_edict(edict_t* edict);

static float s_o2q = 120.f / 2.f;

// From: https://community.khronos.org/t/quake3-coordinate-system/41901
static const FixedArray<float, 3, 3> m_q2o{
    1.f, 0.f, 0.f,
    0.f, 0.f, 1.f,
    0.f, -1.f, 0.f};

static const FixedArray<float, 3, 3> m_o2q = m_q2o.T();

inline FixedArray<float, 3, 3> mm_q2o(const FixedArray<float, 3, 3>& R) {
    return dot2d(m_q2o, dot2d(R, m_o2q));
}

// Position
inline ::Vector p_o2q(const ::Vector& o) {
    return dot1d(m_o2q, o) * s_o2q;
    // return ::Vector{ o(0), -o(2), o(1) } * s_o2q;
}

// Position
inline ::Vector p_q2o(const ::Vector& q) {
    return dot1d(m_q2o, q) / s_o2q;
    // return ::Vector{ o(0), o(2), -o(1) } * s_o2q;
}

// Unit vector
inline ::Vector u_o2q(const ::Vector& o) {
    return dot1d(m_o2q, o);
    // return ::Vector{ o(0), -o(2), o(1) };
}

}
