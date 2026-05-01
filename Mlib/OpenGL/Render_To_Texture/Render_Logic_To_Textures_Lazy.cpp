#include "Render_Logic_To_Textures_Lazy.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/OpenGL/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/OpenGL/Instance_Handles/Render_Guards.hpp>
#include <Mlib/OpenGL/Render_Config.hpp>
#include <Mlib/OpenGL/Render_Logic.hpp>
#include <Mlib/OpenGL/Render_To_Texture/Color_Extrapolation_mode.hpp>
#include <Mlib/OpenGL/Render_To_Texture/Copy_Alpha_Channel/Copy_Alpha_Channel.hpp>
#include <Mlib/OpenGL/Render_To_Texture/Copy_Alpha_Channel/Restore_Alpha_Channel.hpp>
#include <Mlib/OpenGL/Render_To_Texture/Lowpass.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Viewport_Guard.hpp>
#include <Mlib/Scene_Config/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <memory>

using namespace Mlib;

enum class ChannelType {
    COLOR,
    DEPTH
};

class LazyRenderLogicTextureHandle: public ITextureHandle {
public:
    explicit LazyRenderLogicTextureHandle(
        std::shared_ptr<RenderLogic> child_logic,
        std::shared_ptr<FrameBuffer> fb_color[2],
        std::shared_ptr<FrameBuffer> fb_alpha,
        FrameBufferConfig config_color,
        FrameBufferConfig config_alpha,
        float dpi,
        ColorExtrapolationMode color_extrapolation_mode,
        ChannelType channel_type)
        : child_logic_{std::move(child_logic)}
        , fb_color_{fb_color[0], fb_color[1]}
        , fb_alpha_{std::move(fb_alpha)}
        , config_color_{config_color}
        , config_alpha_{config_alpha}
        , dpi_{dpi}
        , color_extrapolation_mode_{color_extrapolation_mode}
        , channel_type_{channel_type}
    {}
    virtual ~LazyRenderLogicTextureHandle() override = default;
    virtual uint32_t handle32() const override {
        return ihandle().handle32();
    }
    virtual uint64_t handle64() const override {
        return ihandle().handle64();
    }
    virtual uint32_t& handle32() override {
        return ihandle().handle32();
    }
    virtual uint64_t& handle64() override {
        return ihandle().handle64();
    }
    virtual bool texture_is_loaded_and_try_preload() override {
        throw std::runtime_error("render_logic_to_textures: 'texture_is_loaded_and_try_preload' not supported");
    }
    virtual void load_gpu() override {
        ihandle();
    }
    virtual TextureTarget target() const override {
        return ihandle().target();
    }
    virtual ColorMode color_mode() const override {
        return ihandle().color_mode();
    }
    virtual MipmapMode mipmap_mode() const override {
        return ihandle().mipmap_mode();
    }
    virtual InterpolationMode magnifying_interpolation_mode() const override {
        return ihandle().magnifying_interpolation_mode();
    }
    virtual WrapMode wrap_modes(size_t i) const override {
        return ihandle().wrap_modes(i);
    }
    virtual FixedArray<float, 4> border_color() const override {
        return config_color_.border_color;
    }
    virtual uint32_t layers() const override {
        return 1;
    }
private:
    ITextureHandle& ihandle() {
        if (!fb_color_[0]->is_configured()) {
            fb_color_[0]->configure(config_color_);
            {
                RenderToFrameBufferGuard fbg{ fb_color_[0] };
                ViewportGuard vg{ config_color_.width, config_color_.height };
                child_logic_->render_toplevel(
                    LayoutConstraintParameters{
                        .dpi = dpi_,
                        .min_pixel = 0.f,
                        .end_pixel = (float)config_color_.width},
                    LayoutConstraintParameters{
                        .dpi = dpi_,
                        .min_pixel = 0.f,
                        .end_pixel = (float)config_color_.height},
                    RenderConfig{},
                    SceneGraphConfig{},
                    nullptr, // render_results
                    RenderedSceneDescriptor{});
                // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
                // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
                // PpmImage::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.ppm");
            }
            if (color_extrapolation_mode_ == ColorExtrapolationMode::ENABLED) {
                if (fb_color_[1] == nullptr) {
                    throw std::runtime_error("Color extrapolation requires a second color buffer");
                }
                if (fb_alpha_ == nullptr) {
                    throw std::runtime_error("Color extrapolation requires an alpha buffer");
                }
                fb_color_[1]->configure(config_color_);
                fb_alpha_->configure(config_alpha_);
                CopyAlphaChannel copy_alpha_channel;
                copy_alpha_channel(
                    config_color_.width,
                    config_color_.height,
                    fb_color_[0],
                    fb_alpha_);
                Lowpass lowpass{
                    {Lowpass::Params1{BoxParameters{}}, Lowpass::Params1{BoxParameters{}}},
                    LowpassFlavor::EXTRAPOLATE};
                size_t target_id = 0;
                lowpass.render(
                    config_color_.width,
                    config_color_.height,
                    lowpass.full_extrapolation_niterations(config_color_.width, config_color_.height),
                    fb_color_,
                    target_id);
                RestoreAlphaChannel restore_alpha_channel;
                restore_alpha_channel(
                    config_color_.width,
                    config_color_.height,
                    fb_alpha_,
                    fb_color_[target_id],
                    fb_color_[1 - target_id]);
                if (target_id == 0) {
                    std::swap(fb_color_[0], fb_color_[1]);
                }
            }
        }
        switch (channel_type_) {
        case ChannelType::COLOR:
            return *fb_color_[0]->texture_color();
        case ChannelType::DEPTH:
            return *fb_color_[0]->texture_depth();
        }
        throw std::runtime_error("Unknown channel type");
    }
    const ITextureHandle& ihandle() const {
        return const_cast<LazyRenderLogicTextureHandle*>(this)->ihandle();
    }
    std::shared_ptr<RenderLogic> child_logic_;
    std::shared_ptr<FrameBuffer> fb_color_[2];
    std::shared_ptr<FrameBuffer> fb_alpha_;
    FrameBufferConfig config_color_;
    FrameBufferConfig config_alpha_;
    float dpi_;
    ColorExtrapolationMode color_extrapolation_mode_;
    ChannelType channel_type_;
};

void Mlib::render_logic_to_textures_lazy(
    const std::shared_ptr<RenderLogic>& child_logic,
    RenderingResources& rendering_resources,
    FrameBufferChannelKind depth_kind,
    const FixedArray<int, 2>& texture_size,
    int nsamples_msaa,
    float dpi,
    ColorExtrapolationMode color_extrapolation_mode,
    VariableAndHash<std::string> color_texture_name,
    VariableAndHash<std::string> depth_texture_name)
{
    std::shared_ptr<FrameBuffer> fb_color[2] = {
        std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION),
        (color_extrapolation_mode == ColorExtrapolationMode::DISABLED)
            ? nullptr
            : std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION)
    };
    std::shared_ptr<FrameBuffer> fb_alpha =
        (color_extrapolation_mode == ColorExtrapolationMode::DISABLED)
            ? nullptr
            : std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
    auto config_color = FrameBufferConfig{
        .width = texture_size(0),
        .height = texture_size(1),
        .color_internal_format = GL_RGBA,
        .color_format = GL_RGBA,
        .color_magnifying_interpolation_mode = InterpolationMode::LINEAR,
        .depth_kind = depth_kind,
        .wrap_s = GL_CLAMP_TO_SOMETHING,
        .wrap_t = GL_CLAMP_TO_SOMETHING,
        .border_color = {0.f, 0.f, 0.f, 0.f},
        .with_mipmaps = true,
        .nsamples_msaa = nsamples_msaa};
    auto config_alpha = FrameBufferConfig{
        .width = texture_size(0),
        .height = texture_size(1),
        .color_internal_format = GL_R8,
        .color_format = GL_RED,
        .color_magnifying_interpolation_mode = InterpolationMode::LINEAR,
        .depth_kind = depth_kind,
        .wrap_s = GL_CLAMP_TO_SOMETHING,
        .wrap_t = GL_CLAMP_TO_SOMETHING,
        .border_color = {0.f, 0.f, 0.f, 0.f},
        .with_mipmaps = true,
        .nsamples_msaa = 1};
    {
        auto color = std::make_shared<LazyRenderLogicTextureHandle>(
            child_logic, fb_color, fb_alpha, config_color, config_alpha, dpi,
            color_extrapolation_mode,
            ChannelType::COLOR);
        auto cm = ColormapWithModifiers{
            .filename = FPath::from_variable_and_hash(color_texture_name),
            .color_mode = ColorMode::RGBA
        }.compute_hash();
        rendering_resources.add_texture_descriptor(std::move(color_texture_name), TextureDescriptor{.color = cm});
        rendering_resources.set_texture(std::move(cm), std::move(color));
    }
    if (depth_kind == FrameBufferChannelKind::TEXTURE) {
        auto depth = std::make_shared<LazyRenderLogicTextureHandle>(
            child_logic, fb_color, fb_alpha, config_color, config_alpha, dpi,
            color_extrapolation_mode,
            ChannelType::DEPTH);
        auto cm = ColormapWithModifiers{
            .filename = FPath::from_variable_and_hash(depth_texture_name),
            .color_mode = ColorMode::GRAYSCALE
        };
        rendering_resources.add_texture_descriptor(std::move(depth_texture_name), TextureDescriptor{.color = cm});
        rendering_resources.set_texture(std::move(cm), std::move(depth));
    }
}
