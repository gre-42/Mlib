#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <string>

namespace Mlib {

struct Material;
enum class ExternalRenderPassType;
enum class InternalRenderPass;

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
        InternalRenderPass internal_render_pass,
        const Material& material) const;
    void unapply_material() const;

    // From: https://stackoverflow.com/questions/46510889
    // Not specifying GLFW_CONTEXT_VERSION_MAJOR/MINOR will select the highest supported version.
    // The actual version can then be obtained using "glGetIntegerv(GL_MAJOR_VERSION/MINOR...)
    // int opengl_major_version = 4;
    // int opengl_minor_version = 4;
    int nsamples_msaa = 1;
    int lightmap_nsamples_msaa = 4;
    int imposter_nsamples_msaa = 4;
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
    bool motion_interpolation = false;
    bool fullscreen = false;
    bool double_buffer = false;
    unsigned int anisotropic_filtering_level = 8;
    bool normalmaps = true;
    bool show_mouse_cursor = true;
    int swap_interval = 1;
    int fullscreen_refresh_rate = 0;
    float draw_distance_add = INFINITY;
    float draw_distance_slop = 10.f;
};

class RenderConfigGuard {
    friend class MaterialRenderConfigGuard;
    RenderConfigGuard(const RenderConfigGuard&) = delete;
    RenderConfigGuard& operator = (const RenderConfigGuard&) = delete;
public:
    RenderConfigGuard(
        const RenderConfig& render_config,
        ExternalRenderPassType external_render_pass_type);
    ~RenderConfigGuard();
private:
    const RenderConfig& render_config_;
    ExternalRenderPassType external_render_pass_type_;
    static THREAD_LOCAL(RenderConfigGuard*) current_;
};

class MaterialRenderConfigGuard {
    MaterialRenderConfigGuard(const MaterialRenderConfigGuard&) = delete;
    MaterialRenderConfigGuard& operator = (const MaterialRenderConfigGuard&) = delete;
public:
    explicit MaterialRenderConfigGuard(
        const Material& material,
        InternalRenderPass internal_render_pass);
    ~MaterialRenderConfigGuard();
private:
    static THREAD_LOCAL(bool) applied_;
};

}
