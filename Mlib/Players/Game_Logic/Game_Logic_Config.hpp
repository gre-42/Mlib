#pragma once
#include <Mlib/Physics/Units.hpp>

namespace Mlib {

struct GameLogicConfig {
    // Vehicles below this distance to the VIP must be invisible during spawning,
    // vehicles above the threshold must be visible for the VIP.
    CompressedScenePos r_spawn_near = (CompressedScenePos)(200 * meters);
    // No vehicles are spawned further away from the VIP.
    CompressedScenePos r_spawn_far = (CompressedScenePos)(400 * meters);
    // Vehicles above this threshold are deleted if they are behind the VIP.
    CompressedScenePos r_delete_near = (CompressedScenePos)(100 * meters);
    // Vehicles above this threshold are deleted if they are invisible to the VIP.
    CompressedScenePos r_delete_far = (CompressedScenePos)(300 * meters);
    CompressedScenePos r_neighbors = (CompressedScenePos)(20 * meters);
    float velocity_after_spawn = 50 * kph;
    float visible_after_spawn_time = 2 * seconds;
    float visible_after_delete_time = 3 * seconds;
    float visible_after_spotted_time = 5 * seconds;
    bool only_terrain = true;
    CompressedScenePos spawn_point_can_be_seen_y_offset = (CompressedScenePos)(2 * meters);
    // The one spawner visits each spawn point BVH every 5 seconds.
    size_t spawn_points_nsubdivisions = 5 * 60;
    // The one spawner visits at most 10 spawn points.
    size_t spawn_points_visited_max = 10;
    CompressedScenePos r_occupied_spawn_point = (CompressedScenePos)(5 * meters);
};

}
