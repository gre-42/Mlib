#pragma once
#include <Mlib/Physics/Units.hpp>

namespace Mlib {

struct GameLogicConfig {
    float r_spawn_near = 200 * meters;
    float r_spawn_far = 400 * meters;
    float r_delete_near = 100 * meters;
    float r_delete_far = 300 * meters;
    float r_neighbors = 20 * meters;
    float visible_after_spawn_time = 2 * s;
    float visible_after_delete_seconds = 3.f;
    float spawn_y_offset = 0.7f * meters;
    bool only_terrain = true;
    float can_see_y_offset = 2 * meters;
    size_t spawn_points_nsubdivisions = 5 * 60;
};

}
