#include "Audio_Listener_Updater.hpp"
#include <Mlib/Audio/Audio_Scene.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

AudioListenerUpdater::AudioListenerUpdater(
    const SelectedCameras& selected_cameras,
    const Scene& scene,
    std::chrono::steady_clock::duration delay,
    std::chrono::steady_clock::duration velocity_dt)
    : selected_cameras_{ selected_cameras }
    , scene_{ scene }
    , delay_{ delay }
    , velocity_dt_{ velocity_dt }
{}

void AudioListenerUpdater::advance_time(float dt, const StaticWorld& world) {
    DanglingBaseClassRef<SceneNode> node = selected_cameras_.camera(DP_LOC).node;
    auto corrected_time = std::chrono::steady_clock::now() - delay_;
    AudioScene::set_listener(node, corrected_time, velocity_dt_);
}
