#pragma once
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>

namespace Mlib {

class GenericPostProcessingLogic {
public:
    GenericPostProcessingLogic();
    ~GenericPostProcessingLogic();
protected:
    static const char* simple_vertex_shader_text_;
    VertexArray& va();
private:
    VertexArray va_;
};

}
