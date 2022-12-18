#pragma once
#include <Mlib/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class RenderProgram {
    RenderProgram(const RenderProgram&) = delete;
    RenderProgram& operator = (const RenderProgram&) = delete;
public:
    RenderProgram();
    ~RenderProgram();
    GLuint vertex_shader = (GLuint)-1;
    GLuint fragment_shader = (GLuint)-1;
    GLuint program = (GLuint)-1;
    bool allocated() const;
    void allocate(const char * vertex_shader_text, const char * fragment_shader_text);
    void deallocate();
    void gc_deallocate();
private:
    DeallocationToken deallocation_token_;
};

}
