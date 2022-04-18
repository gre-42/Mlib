#include "Render_Config.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

void RenderConfig::apply(ExternalRenderPassType external_render_pass_type) const {
    if ((external_render_pass_type == ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) ||
        (external_render_pass_type == ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC) ||
        (external_render_pass_type == ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) ||
        (external_render_pass_type == ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES) ||
        (external_render_pass_type == ExternalRenderPassType::LIGHTMAP_BLACK_NODE))
    {
        CHK(glEnable(GL_CULL_FACE));
        if (lightmap_nsamples_msaa == 0) {
            throw std::runtime_error("lightmap_nsamples_msaa must be >= 1");
        }
        if (lightmap_nsamples_msaa != 1) {
            CHK(glEnable(GL_MULTISAMPLE));
        }
    } else if ((external_render_pass_type == ExternalRenderPassType::STANDARD) ||
               (external_render_pass_type == ExternalRenderPassType::DIRTMAP)) {
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
    } else {
        throw std::runtime_error("RenderConfig::apply: unknown render pass type");
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

thread_local bool RenderConfigGuard::applied_ = false;

RenderConfigGuard::RenderConfigGuard(const RenderConfig& render_config, ExternalRenderPassType external_render_pass_type)
: render_config_{ render_config }
{
    if (applied_) {
        throw std::runtime_error("Detected recursive application of render config");
    }
    applied_ = true;
    render_config.apply(external_render_pass_type);
}

RenderConfigGuard::~RenderConfigGuard() {
    applied_ = false;
    render_config_.unapply();
}