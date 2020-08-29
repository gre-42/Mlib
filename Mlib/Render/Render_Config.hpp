#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <string>

namespace Mlib {

struct RenderConfig {
    int nsamples_msaa = 1;
    bool cull_faces = true;
    bool wire_frame = false;
    std::string window_title = "Simple example";
    int screen_width = 640;
    int screen_height = 480;
    int scene_lightmap_width = 2048;
    int scene_lightmap_height = 2048;
    int black_lightmap_width = 1024;
    int black_lightmap_height = 1024;
    bool motion_interpolation = false;
    bool full_screen = false;
    bool window_maximized = false;
    bool show_mouse_cursor = false;
    int swap_interval = 1;
    FixedArray<float, 3> background_color = fixed_zeros<float, 3>();
    bool print_fps = false;
    float dt = 0.01667;
    float max_residual_time = 0.5;
    void apply() const;
    void unapply() const;
};

}
