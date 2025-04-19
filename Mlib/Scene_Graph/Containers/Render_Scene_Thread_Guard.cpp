#include "Render_Scene_Thread_Guard.hpp"
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

RenderSceneThreadGuard::RenderSceneThreadGuard(const Scene& scene)
    : scene_{ const_cast<Scene&>(scene) }
{
    scene_.set_this_thread_as_render_thread();
}

RenderSceneThreadGuard::~RenderSceneThreadGuard() {
    scene_.clear_render_thread();
}
