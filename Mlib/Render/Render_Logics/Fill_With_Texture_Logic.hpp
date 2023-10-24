#pragma once
#include <Mlib/Geometry/Material/Cull_Face_Mode.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <string>

namespace Mlib {

class RenderingResources;
enum class ResourceUpdateCycle;
struct LayoutConstraintParameters;
enum class ColorMode;

class FillWithTextureRenderProgram: public RenderProgram {
    FillWithTextureRenderProgram(const FillWithTextureRenderProgram&) = delete;
    FillWithTextureRenderProgram& operator = (const FillWithTextureRenderProgram&) = delete;
public:
    FillWithTextureRenderProgram();
    ~FillWithTextureRenderProgram();
    GLint texture_location = -1;
    GLuint texture_id_ = (GLuint)-1;
private:
    void deallocate();
    DeallocationToken deallocation_token_;
};

class FillWithTextureLogic: public GenericPostProcessingLogic {
public:
    FillWithTextureLogic(
        RenderingResources& rendering_resources,
        std::string image_resource_name,
        ResourceUpdateCycle update_cycle,
        ColorMode color_mode,
        CullFaceMode cull_face_mode = CullFaceMode::CULL,
        const float* quad_vertices = standard_quad_vertices);
    ~FillWithTextureLogic();
    void set_image_resource_name(const std::string& image_resource_name);
    void update_texture_id();
    bool texture_is_loaded_and_try_preload() const;
    void render();
    void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly);

private:
    FillWithTextureRenderProgram rp_;
    RenderingResources& rendering_resources_;
    std::string image_resource_name_;
    ResourceUpdateCycle update_cycle_;
    ColorMode color_mode_;
    CullFaceMode cull_face_mode_;
};

}
