#pragma once
#include <Mlib/Physics/Units.hpp>

namespace Mlib {

struct GameLogicConfig {
    CompressedScenePos r_spawn_near = (CompressedScenePos)(200 * meters);
    CompressedScenePos r_spawn_far = (CompressedScenePos)(400 * meters);
    CompressedScenePos r_delete_near = (CompressedScenePos)(100 * meters);
    CompressedScenePos r_delete_far = (CompressedScenePos)(300 * meters);
    CompressedScenePos r_neighbors = (CompressedScenePos)(20 * meters);
    float visible_after_spawn_time = 2 * seconds;
    float visible_after_delete_time = 3 * seconds;
    bool only_terrain = true;
    CompressedScenePos spawn_point_can_be_seen_y_offset = (CompressedScenePos)(2 * meters);
    size_t spawn_points_nsubdivisions = 5 * 60;
    CompressedScenePos r_occupied_spawn_point = (CompressedScenePos)(5 * meters);
};

}
