#pragma once
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Empty_Array_Buffer.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

class RenderingResources;

struct SRenderProgram: public RenderProgram {
    GLint skybox_location = -1;
    GLint vp_location = -1;
};

class SkyboxLogic: public RenderLogic {
public:
    explicit SkyboxLogic(RenderLogic& child_logic);
    ~SkyboxLogic();

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<ScenePos, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, ScenePos, 3>& iv() const override;
    virtual DanglingRef<const SceneNode> camera_node() const override;
    virtual bool requires_postprocessing() const override;
    virtual void reset() override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void clear_alias();
    void set_alias(const std::string& alias);
private:
    void deallocate();
    RenderLogic& child_logic_;
    RenderingResources& rendering_resources_;
    SRenderProgram rp_;
    BufferBackgroundCopy vertices_;
    EmptyArrayBuffer empty_;
    VertexArray va_;
    std::string alias_;
    bool loaded_;
    ColormapWithModifiers colormap_;
    DeallocationToken deallocation_token_;
};

}
