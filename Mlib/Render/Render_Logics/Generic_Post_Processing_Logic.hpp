#pragma once
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class GenericPostProcessingLogic: public RenderLogic {
public:
    virtual void initialize(GLFWwindow* window) override;
protected:
    static const char* vertex_shader_text;
    VertexArray va_;
};

}
