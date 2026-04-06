#pragma once
#include <Mlib/OpenGL/Instance_Handles/Render_Program.hpp>
#include <Mlib/OpenGL/Render_Logics/Generic_Post_Processing_Logic.hpp>

namespace Mlib {

class FrameBuffer;

struct RestoreAlphaChannelRenderProgram: public RenderProgram {
    GLint texture_color_location = -1;
    GLint texture_alpha_location = -1;
};

class RestoreAlphaChannel: public GenericPostProcessingLogic {
public:
    RestoreAlphaChannel();
    ~RestoreAlphaChannel();
    void operator () (
        int width,
        int height,
        const std::shared_ptr<FrameBuffer>& alpha,
        const std::shared_ptr<FrameBuffer>& color_source,
        const std::shared_ptr<FrameBuffer>& color_dest);
private:
    RestoreAlphaChannelRenderProgram rp_;
};

}
