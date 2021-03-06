#include "Render_Config.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

void RenderConfig::apply(ExternalRenderPassType external_render_pass_type) const {
    if (bool(external_render_pass_type & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        CHK(glEnable(GL_CULL_FACE));
        if (lightmap_nsamples_msaa == 0) {
            throw std::runtime_error("lightmap_nsamples_msaa must be >= 1");
        }
        if (lightmap_nsamples_msaa != 1) {
            CHK(glEnable(GL_MULTISAMPLE));
        }
    } else {
        if (cull_faces == BoolRenderOption::ON) {
            CHK(glEnable(GL_CULL_FACE));
        }
        if (wire_frame == BoolRenderOption::ON) {
            CHK(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
        }
        if (depth_test == BoolRenderOption::ON) {
            CHK(glEnable(GL_DEPTH_TEST));
        }
        if (min_sample_shading != 0) {
            CHK(glEnable(GL_SAMPLE_SHADING));
            CHK(glMinSampleShading(min_sample_shading));
        }
        if (nsamples_msaa == 0) {
            throw std::runtime_error("nsamples_msaa must be >= 1");
        }
        if (nsamples_msaa != 1) {
            CHK(glEnable(GL_MULTISAMPLE));
        }
    }
}

void RenderConfig::apply_material(
    ExternalRenderPassType external_render_pass_type,
    const Material& material) const
{
    if ((cull_faces != BoolRenderOption::OFF) && material.cull_faces) {
        CHK(glEnable(GL_CULL_FACE));
    }
    if (bool(external_render_pass_type & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
        CHK(glEnable(GL_BLEND));
        CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
        CHK(glDepthMask(GL_FALSE));
    } else if (bool(external_render_pass_type & ExternalRenderPassType::LIGHTMAP_COLOR_MASK)) {
        CHK(glEnable(GL_BLEND));
        CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        CHK(glDepthMask(GL_FALSE));
    } else {
        if ((depth_test != BoolRenderOption::OFF) && material.depth_test) {
            CHK(glEnable(GL_DEPTH_TEST));
        }
        if (!bool(external_render_pass_type & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK)) {
            switch(material.blend_mode) {
                case BlendMode::OFF:
                case BlendMode::BINARY:
                    break;
                case BlendMode::BINARY_ADD:
                    CHK(glEnable(GL_BLEND));
                    CHK(glBlendFunc(GL_ONE, GL_ONE));
                    CHK(glDepthMask(GL_FALSE));
                    break;
                case BlendMode::SEMI_CONTINUOUS:
                    CHK(glEnable(GL_BLEND));
                    CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                    break;
                case BlendMode::CONTINUOUS:
                    CHK(glEnable(GL_BLEND));
                    CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                    CHK(glDepthMask(GL_FALSE));
                    break;
                default:
                    throw std::runtime_error("Unknown blend_mode in material: " + material.identifier());
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
                    throw std::runtime_error("Unknown depth func in material: " + material.identifier());
            }
        }
    }
}

void RenderConfig::unapply() const {
    CHK(glDisable(GL_CULL_FACE));
    CHK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    CHK(glDisable(GL_DEPTH_TEST));
    CHK(glDisable(GL_SAMPLE_SHADING));
    CHK(glMinSampleShading(0.f));
    CHK(glDisable(GL_MULTISAMPLE));
}

void RenderConfig::unapply_material() const {
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_DEPTH_TEST));
    CHK(glDisable(GL_BLEND));
    CHK(glBlendFunc(GL_ONE, GL_ZERO));
    CHK(glDepthMask(GL_TRUE));
    CHK(glDepthFunc(GL_LESS));
}

thread_local RenderConfigGuard* RenderConfigGuard::current_ = nullptr;

RenderConfigGuard::RenderConfigGuard(
    const RenderConfig& render_config,
    ExternalRenderPassType external_render_pass_type)
: render_config_{ render_config },
  external_render_pass_type_{ external_render_pass_type }
{
    if (current_ != nullptr) {
        throw std::runtime_error("Detected recursive application of render config");
    }
    current_ = this;
    render_config.apply(external_render_pass_type);
}

RenderConfigGuard::~RenderConfigGuard() {
    current_ = nullptr;
    render_config_.unapply();
}

thread_local bool MaterialRenderConfigGuard::applied_ = false;

MaterialRenderConfigGuard::MaterialRenderConfigGuard(const Material& material) {
    if (applied_) {
        throw std::runtime_error("Detected recursive application of material render config");
    }
    if (RenderConfigGuard::current_ == nullptr) {
        throw std::runtime_error("Material render guard without render guard");
    }
    applied_ = true;
    RenderConfigGuard::current_->render_config_.apply_material(RenderConfigGuard::current_->external_render_pass_type_, material);
}

MaterialRenderConfigGuard::~MaterialRenderConfigGuard() {
    applied_ = false;
    RenderConfigGuard::current_->render_config_.unapply_material();
}
