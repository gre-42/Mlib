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
        glEnable(GL_MULTISAMPLE);
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
        glDisable(GL_MULTISAMPLE);
    }
}
