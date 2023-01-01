#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_Channel_Kind.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <map>

namespace Mlib {

struct RenderResult {
    int width = 1024;
    int height = 512;
    bool flip_y = true;
    FrameBufferChannelKind depth_kind = FrameBufferChannelKind::ATTACHMENT;
    Array<float> rgb = Array<float>();
    Array<float> depth = Array<float>();
};

struct RenderResults {
    Array<float>* output = nullptr;
    std::map<RenderedSceneDescriptor, RenderResult> outputs;
};

}
