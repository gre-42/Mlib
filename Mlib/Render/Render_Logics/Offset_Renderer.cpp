#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Instance_Handles/Texture_Binder.hpp>
#include <Mlib/Render/Render_Logics/Offset_Renderer.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <sstream>

using namespace Mlib;

OffsetRenderProgram::OffsetRenderProgram() = default;
OffsetRenderProgram::~OffsetRenderProgram() = default;

OffsetRenderer::OffsetRenderer(size_t ndim)
    : ndim_{ ndim }
{}

void OffsetRenderer::apply_offset(
    const FixedArray<float, 2>& offset,
    int width,
    int height,
    std::shared_ptr<FrameBuffer>& field,
    std::shared_ptr<FrameBuffer>& temporary_field)
{
    if (!rp_.allocated()) {
        std::stringstream vs;
        vs << SHADER_VER;
        vs << "layout (location = 0) in vec2 aPos;" << std::endl;
        vs << "layout (location = 1) in vec2 aTexCoords;" << std::endl;
        vs << std::endl;
        vs << "out vec2 TexCoords0;" << std::endl;
        vs << "uniform vec2 offset;" << std::endl;
        vs << std::endl;
        vs << "void main()" << std::endl;
        vs << "{" << std::endl;
        vs << "    TexCoords0 = aTexCoords - offset;" << std::endl;
        vs << "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);" << std::endl;
        vs << "}" << std::endl;
        
        std::stringstream fs;
        {
            if ((ndim_ < 1) || (ndim_ > 4)) {
                THROW_OR_ABORT("OffsetRenderer: Number of dimensions out of bounds");
            }
            const char* coords = "rgba";
            fs << SHADER_VER << FRAGMENT_PRECISION;
            if (ndim_ == 1) {
                fs << "out float result;" << std::endl;
            } else {
                fs << "out vec" << ndim_ << " result;" << std::endl;
            }
            fs << "in vec2 TexCoords0;" << std::endl;
            fs << "uniform sampler2D field;" << std::endl;
            fs << "void main() {" << std::endl;
            fs << "    result = texture(field, TexCoords0)." << std::string(coords, ndim_) << ';' << std::endl;
            fs << "}" << std::endl;
        }
        // linfo() << "--------- apply_offset -----------";
        // lraw() << vs.str();
        // lraw() << fs.str();
        rp_.allocate(vs.str().c_str(), fs.str().c_str());
        rp_.offset = rp_.get_uniform_location("offset");
        rp_.field = rp_.get_uniform_location("field");
    }
    rp_.use();
    CHK(glUniform2fv(rp_.offset, 1, offset.flat_begin()));
    ViewportGuard vg{ width, height };
    {
        RenderToFrameBufferGuard rfg{ temporary_field };

        notify_rendering(CURRENT_SOURCE_LOCATION);
        TextureBinder tb;
        tb.bind(rp_.field, *field->texture_color());
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        va().bind();
        CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
        CHK(glBindVertexArray(0));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
    std::swap(temporary_field, field);
}
