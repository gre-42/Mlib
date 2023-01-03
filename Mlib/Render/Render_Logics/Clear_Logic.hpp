#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <mutex>

namespace Mlib {

enum class ClearMode;

struct ClearRenderProgram: public RenderProgram {
    GLint clear_color_location = -1;
};

class ClearLogic {
public:
    ClearLogic();
    ~ClearLogic();

    void clear_color(const FixedArray<float, 4>& color);
    void clear_depth();
    void clear_color_and_depth(const FixedArray<float, 4>& color);
private:
    void ensure_va_initialized();
    VertexArray va_;
    ClearRenderProgram rp_color_only_;
    RenderProgram rp_depth_only_;
    ClearRenderProgram rp_color_and_depth_;
    std::mutex mutex_;
};

}
