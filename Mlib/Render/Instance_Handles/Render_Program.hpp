#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class RenderProgram {
    RenderProgram(const RenderProgram&) = delete;
    RenderProgram& operator = (const RenderProgram&) = delete;
public:
    RenderProgram();
    ~RenderProgram();
    bool allocated() const;
    void allocate(const char* vertex_shader_text, const char* fragment_shader_text);
    void deallocate();
    void gc_deallocate();
    void use() const;
    GLint get_uniform_location(const char* name) const;
private:
    GLuint vertex_shader_ = 0;
    GLuint fragment_shader_ = 0;
    GLuint program_ = 0;
    DeallocationToken deallocation_token_;
};

}
