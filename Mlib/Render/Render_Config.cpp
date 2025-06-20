#include "Render_Config.hpp"
#include <Mlib/Features.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>

using namespace Mlib;

void RenderConfig::apply(ExternalRenderPassType external_render_pass_type) const {
    if (any(external_render_pass_type & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        CHK(glEnable(GL_CULL_FACE));
        if (lightmap_nsamples_msaa == 0) {
            THROW_OR_ABORT("lightmap_nsamples_msaa must be >= 1");
        }
        if (lightmap_nsamples_msaa != 1) {
#ifdef __ANDROID__
            THROW_OR_ABORT("MSAA not supported on Android");
#else
            CHK(glEnable(GL_MULTISAMPLE));
#endif
        }
    } else {
        if (cull_faces == BoolRenderOption::ON) {
            CHK(glEnable(GL_CULL_FACE));
        }
        if (wire_frame == BoolRenderOption::ON) {
#ifdef __ANDROID__
            THROW_OR_ABORT("Wireframe rasterization not supported on Android");
#else
            CHK(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
#endif
        }
        if (depth_test == BoolRenderOption::ON) {
            CHK(glEnable(GL_DEPTH_TEST));
        }
        if (min_sample_shading != 0) {
#ifdef __ANDROID__
            THROW_OR_ABORT("Min sample shading not supported on Android");
#else
            CHK(glEnable(GL_SAMPLE_SHADING));
            CHK(glMinSampleShading(min_sample_shading));
#endif
        }
        if (nsamples_msaa == 0) {
            THROW_OR_ABORT("nsamples_msaa must be >= 1");
        }
        if (nsamples_msaa != 1) {
#ifdef __ANDROID__
            THROW_OR_ABORT("MSAA not supported on Android");
#else
            CHK(glEnable(GL_MULTISAMPLE));
#endif
        }
    }
}

void RenderConfig::apply_material(
    ExternalRenderPassType external_render_pass_type,
    InternalRenderPass internal_render_pass,
    const Material& material) const
{
    if ((cull_faces != BoolRenderOption::OFF) && material.cull_faces) {
        CHK(glEnable(GL_CULL_FACE));
    }
    if (any(external_render_pass_type & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
        CHK(glEnable(GL_BLEND));
        CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
        CHK(glDepthMask(GL_FALSE));
    } else if (any(external_render_pass_type & ExternalRenderPassType::LIGHTMAP_COLOR_MASK)) {
        CHK(glEnable(GL_BLEND));
        CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        CHK(glDepthMask(GL_FALSE));
    } else {
        if ((depth_test != BoolRenderOption::OFF) && material.depth_test) {
            CHK(glEnable(GL_DEPTH_TEST));
        }
        if (!any(external_render_pass_type & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK)) {
            switch(material.blend_mode) {
                case BlendMode::OFF:
                case BlendMode::BINARY_05:
                case BlendMode::BINARY_08:
                    break;
                case BlendMode::BINARY_05_ADD:
                case BlendMode::CONTINUOUS_ADD:
                    CHK(glEnable(GL_BLEND));
                    CHK(glBlendFunc(GL_ONE, GL_ONE));
                    CHK(glDepthMask(GL_FALSE));
                    break;
                case BlendMode::SEMI_CONTINUOUS_02:
                case BlendMode::SEMI_CONTINUOUS_08:
                    CHK(glEnable(GL_BLEND));
                    if (external_render_pass_type == ExternalRenderPassType::IMPOSTER_NODE) {
                        // From: https://stackoverflow.com/questions/2171085/opengl-blending-with-previous-contents-of-framebuffer
                        CHK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
                    } else {
                        CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                    }
                    break;
                case BlendMode::CONTINUOUS:
                    CHK(glEnable(GL_BLEND));
                    if (external_render_pass_type == ExternalRenderPassType::IMPOSTER_NODE) {
                        // From: https://stackoverflow.com/questions/2171085/opengl-blending-with-previous-contents-of-framebuffer
                        CHK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
                    } else {
                        CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                    }
                    if (internal_render_pass != InternalRenderPass::BLENDED_EARLY) {
                        CHK(glDepthMask(GL_FALSE));
                    }
                    break;
                default:
                    THROW_OR_ABORT("Unknown blend_mode in material: " + material.identifier());
            }
            switch(material.depth_func) {
                case DepthFunc::LESS:
                    break;
                case DepthFunc::EQUAL:
                    CHK(glDepthFunc(GL_EQUAL));
                    break;
                case DepthFunc::LESS_EQUAL:
                    CHK(glDepthFunc(GL_LEQUAL));
                    break;
                default:
                    THROW_OR_ABORT("Unknown depth func in material: " + material.identifier());
            }
        }
    }
}

void RenderConfig::unapply() const {
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_DEPTH_TEST));
#ifndef __ANDROID__
    CHK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    CHK(glDisable(GL_SAMPLE_SHADING));
    CHK(glMinSampleShading(0.f));
    CHK(glDisable(GL_MULTISAMPLE));
#endif
}

void RenderConfig::unapply_material() const {
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_DEPTH_TEST));
    CHK(glDisable(GL_BLEND));
    CHK(glBlendFunc(GL_ONE, GL_ZERO));
    CHK(glDepthMask(GL_TRUE));
    CHK(glDepthFunc(GL_LESS));
}

THREAD_LOCAL(RenderConfigGuard*) RenderConfigGuard::current_ = nullptr;

RenderConfigGuard::RenderConfigGuard(
    const RenderConfig& render_config,
    ExternalRenderPassType external_render_pass_type)
    : render_config_{ render_config }
    , external_render_pass_type_{ external_render_pass_type }
{
    if (current_ != nullptr) {
        THROW_OR_ABORT("Detected recursive application of render config");
    }
    current_ = this;
    render_config.apply(external_render_pass_type);
}

RenderConfigGuard::~RenderConfigGuard() {
    current_ = nullptr;
    render_config_.unapply();
}

THREAD_LOCAL(bool) MaterialRenderConfigGuard::applied_ = false;

MaterialRenderConfigGuard::MaterialRenderConfigGuard(
    const Material& material,
    InternalRenderPass internal_render_pass)
{
    if (applied_) {
        THROW_OR_ABORT("Detected recursive application of material render config");
    }
    if (RenderConfigGuard::current_ == nullptr) {
        THROW_OR_ABORT("Material render guard without render guard");
    }
    applied_ = true;
    RenderConfigGuard::current_->render_config_.apply_material(
        RenderConfigGuard::current_->external_render_pass_type_,
        internal_render_pass,
        material);
}

MaterialRenderConfigGuard::~MaterialRenderConfigGuard() {
    applied_ = false;
    RenderConfigGuard::current_->render_config_.unapply_material();
}
