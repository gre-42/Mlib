#include "Render_Config.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

void RenderConfig::apply() const {
    if (cull_faces) {
        CHK(glEnable(GL_CULL_FACE));
    }
    if (wire_frame) {
        CHK(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
    }
    CHK(glEnable(GL_DEPTH_TEST));
    if (nsamples_msaa != 1) {
        CHK(glEnable(GL_MULTISAMPLE));
    }
}

void RenderConfig::unapply() const {
    if (cull_faces) {
        CHK(glDisable(GL_CULL_FACE));
    }
    if (wire_frame) {
        CHK(glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ));
    }
    CHK(glDisable(GL_DEPTH_TEST));
    if (nsamples_msaa != 1) {
        CHK(glDisable(GL_MULTISAMPLE));
    }
}

thread_local bool RenderConfigGuard::applied_ = false;

RenderConfigGuard::RenderConfigGuard(const RenderConfig& render_config)
: render_config_{ render_config }
{
    if (applied_) {
        throw std::runtime_error("Detected recursive application of render config");
    }
    applied_ = true;
    render_config.apply();
}

RenderConfigGuard::~RenderConfigGuard() {
    applied_ = false;
    render_config_.unapply();
}