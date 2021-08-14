#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <map>

namespace Mlib {

struct RenderResult {
    bool flip_y = true;
    Array<float> rgb = Array<float>();
    Array<float> depth = Array<float>();
};

struct RenderResults {
    Array<float>* output = nullptr;
    std::map<RenderedSceneDescriptor, RenderResult> outputs;
};

}
