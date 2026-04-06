#pragma once
#include <Mlib/OpenGL/Instance_Handles/Render_Program.hpp>
#include <Mlib/OpenGL/Render_Logics/Generic_Post_Processing_Logic.hpp>

namespace Mlib {

class FrameBuffer;

struct CopyAlphaChannelRenderProgram: public RenderProgram {
    GLint texture_color_location = -1;
};

class CopyAlphaChannel: public GenericPostProcessingLogic {
public:
    CopyAlphaChannel();
    ~CopyAlphaChannel();
    void operator () (
        int width,
        int height,
        const std::shared_ptr<FrameBuffer>& color,
        const std::shared_ptr<FrameBuffer>& alpha);
private:
    CopyAlphaChannelRenderProgram rp_;
};

}
