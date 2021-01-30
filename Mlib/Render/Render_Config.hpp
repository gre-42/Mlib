#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <string>

namespace Mlib {

struct RenderConfig {
    int nsamples_msaa = 1;
    int lightmap_nsamples_msaa = 4;
    bool cull_faces = true;
    bool wire_frame = false;
    std::string window_title = "Simple example";
    int screen_width = 640;
    int screen_height = 480;
    int scene_lightmap_width = 2048;
    int scene_lightmap_height = 2048;
    int black_lightmap_width = 512;
    int black_lightmap_height = 512;
    bool motion_interpolation = false;
    bool window_maximized = false;
    bool full_screen = false;
    bool double_buffer = false;
    bool show_mouse_cursor = false;
    int swap_interval = 1;
    FixedArray<float, 3> background_color = fixed_zeros<float, 3>();
    bool print_fps = false;
    bool print_residual_time = false;
    float dt = 0.01667f;
    float max_residual_time = 0.5f;
    float draw_distance_add = INFINITY;
    float draw_distance_slop = 10;
    void apply() const;
    void unapply() const;
};

}
