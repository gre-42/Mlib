#pragma once
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>

namespace Mlib {

class GenericPostProcessingLogic {
public:
    GenericPostProcessingLogic();
protected:
    static const char* vertex_shader_text;
    VertexArray va_;
};

}
