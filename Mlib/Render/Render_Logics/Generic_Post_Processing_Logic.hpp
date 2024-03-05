#pragma once
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logics/Textured_Quad_Style.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Empty_Array_Buffer.hpp>

namespace Mlib {

class GenericPostProcessingLogic {
public:
    explicit GenericPostProcessingLogic(const float* quad_vertices = standard_quad_vertices);
    ~GenericPostProcessingLogic();
protected:
    static const char* simple_vertex_shader_text_;
    VertexArray& va();
private:
    BufferBackgroundCopy vertices_;
    EmptyArrayBuffer empty_;
    VertexArray va_;
    const float* quad_vertices_;
};

}
