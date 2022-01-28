#pragma once

namespace Mlib {

struct GameLogicConfig {
    float r_spawn_near = 200;
    float r_spawn_far = 400;
    float r_delete_near = 100;
    float r_delete_far = 300;
    float r_neighbors = 20;
    float visible_after_spawn = 2;
    float visible_after_delete = 3;
    float spawn_y_offset = 0.7f;
    bool only_terrain = true;
    float can_see_y_offset = 2;
    size_t spawn_points_nsubdivisions = 5 * 60;
};

}
