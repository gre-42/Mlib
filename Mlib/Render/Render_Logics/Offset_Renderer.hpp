#pragma once
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class FrameBuffer;

struct OffsetRenderProgram: public RenderProgram {
    OffsetRenderProgram(const OffsetRenderProgram&) = delete;
    OffsetRenderProgram& operator = (const OffsetRenderProgram&) = delete;
public:
    OffsetRenderProgram();
    ~OffsetRenderProgram();
    GLint offset = -1;
    GLint field = -1;
};

class OffsetRenderer: private GenericPostProcessingLogic {
public:
    explicit OffsetRenderer(size_t ndim);
    void apply_offset(
        const FixedArray<float, 2>& offset,
        int width,
        int height,
        std::shared_ptr<FrameBuffer>& field,
        std::shared_ptr<FrameBuffer>& temporary_field);
private:
    size_t ndim_;
    OffsetRenderProgram rp_;
};

}
