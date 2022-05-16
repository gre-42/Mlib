#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <string>

namespace Mlib {

struct Material;

struct NoCopy {
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator = (const NoCopy&) = delete;
};

enum class BoolRenderOption {
    UNCHANGED = 0,
    OFF = 1,
    ON = 2
};

inline BoolRenderOption zapped(BoolRenderOption option) {
    return (BoolRenderOption)(((int)option + 1) % 3);
}

struct RenderConfig {
    NoCopy no_copy;
    void apply(ExternalRenderPassType external_render_pass_type) const;
    void unapply() const;

    void apply_material(
        ExternalRenderPassType external_render_pass_type,
        const Material& material) const;
    void unapply_material() const;

    int opengl_major_version = 4;
    int opengl_minor_version = 0;
    int nsamples_msaa = 1;
    int lightmap_nsamples_msaa = 4;
    float min_sample_shading = 0.f;
    bool vfx = false;
    bool fxaa = false;
    BoolRenderOption cull_faces = BoolRenderOption::UNCHANGED;
    BoolRenderOption wire_frame = BoolRenderOption::UNCHANGED;
    BoolRenderOption depth_test = BoolRenderOption::UNCHANGED;
    std::string window_title = "Simple example";
    int windowed_width = 640;
    int windowed_height = 480;
    int fullscreen_width = 0;
    int fullscreen_height = 0;
    int scene_lightmap_width = 2048;
    int scene_lightmap_height = 2048;
    int black_lightmap_width = 512;
    int black_lightmap_height = 512;
    bool motion_interpolation = false;
    bool fullscreen = false;
    bool double_buffer = false;
    unsigned int anisotropic_filtering_level = 8;
    bool normalmaps = true;
    bool show_mouse_cursor = true;
    int swap_interval = 1;
    FixedArray<float, 3> background_color = fixed_zeros<float, 3>();
    bool print_fps = false;
    bool control_fps = true;
    bool print_residual_time = false;
    float dt = 0.01667f;
    float max_residual_time = 0.5f;
    float draw_distance_add = INFINITY;
    float draw_distance_slop = 10.f;
};

class RenderConfigGuard {
    friend class MaterialRenderConfigGuard;
public:
    RenderConfigGuard(
        const RenderConfig& render_config,
        ExternalRenderPassType external_render_pass_type);
    ~RenderConfigGuard();
private:
    const RenderConfig& render_config_;
    ExternalRenderPassType external_render_pass_type_;
    static thread_local RenderConfigGuard* current_;
};

class MaterialRenderConfigGuard {
public:
    explicit MaterialRenderConfigGuard(const Material& material);
    ~MaterialRenderConfigGuard();
private:
    static thread_local bool applied_;
};

}
