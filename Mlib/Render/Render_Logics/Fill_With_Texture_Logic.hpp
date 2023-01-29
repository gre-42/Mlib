#pragma once
#include <Mlib/Deallocation_Token.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <memory>
#include <string>

namespace Mlib {

class RenderingResources;
enum class ResourceUpdateCycle;

class FillWithTextureRenderProgram: public RenderProgram {
    FillWithTextureRenderProgram(const FillWithTextureRenderProgram&) = delete;
    FillWithTextureRenderProgram& operator = (const FillWithTextureRenderProgram&) = delete;
public:
    FillWithTextureRenderProgram();
    ~FillWithTextureRenderProgram();
    GLint texture_location = -1;
    GLuint texture_id_ = (GLuint)-1;
private:
    void deallocate();
    DeallocationToken deallocation_token_;
};

class FillWithTextureLogic: public GenericPostProcessingLogic {
public:
    FillWithTextureLogic(
        std::string image_resource_name,
        ResourceUpdateCycle update_cycle);
    ~FillWithTextureLogic();

    void update_texture_id();

    void render();

protected:
    FillWithTextureRenderProgram rp_;
    std::shared_ptr<RenderingResources> rendering_resources_;
    std::string image_resource_name_;
    ResourceUpdateCycle update_cycle_;
};

}
