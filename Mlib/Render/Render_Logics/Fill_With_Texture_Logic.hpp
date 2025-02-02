#pragma once
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Material/Cull_Face_Mode.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <optional>
#include <string>

namespace Mlib {

class ITextureHandle;
struct LayoutConstraintParameters;
enum class MipmapMode;
enum class ClearMode;

enum class ContinuousBlendMode {
    NONE,
    ALPHA,
    ADD
};

struct FillWithTextureRenderProgram: public RenderProgram {
    FillWithTextureRenderProgram(const FillWithTextureRenderProgram&) = delete;
    FillWithTextureRenderProgram& operator = (const FillWithTextureRenderProgram&) = delete;
public:
    FillWithTextureRenderProgram();
    ~FillWithTextureRenderProgram();
    GLint texture_location = -1;
};

class FillWithTextureLogic: public GenericPostProcessingLogic {
public:
    FillWithTextureLogic(
        std::shared_ptr<ITextureHandle> texture,
        CullFaceMode cull_face_mode = CullFaceMode::CULL,
        ContinuousBlendMode blend_mode = ContinuousBlendMode::ALPHA,
        const float* quad_vertices = standard_quad_vertices,
        std::optional<size_t> layer = std::nullopt);
    ~FillWithTextureLogic();
    void set_image_resource_name(std::shared_ptr<ITextureHandle> texture);
    bool texture_is_loaded_and_try_preload() const;
    void render_wo_update_and_bind();
    void render(ClearMode clear_mode);
    void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly);

private:
    void ensure_allocated();
    FillWithTextureRenderProgram rp_;
    std::shared_ptr<ITextureHandle> texture_;
    CullFaceMode cull_face_mode_;
    ContinuousBlendMode blend_mode_;
    std::optional<size_t> layer_;
};

}
