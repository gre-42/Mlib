#include "Audio_Listener_Updater.hpp"
#include <Mlib/Audio/Audio_Scene.hpp>
#include <Mlib/OpenGL/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

AudioListenerUpdater::AudioListenerUpdater(
    const SelectedCameras& selected_cameras,
    std::chrono::steady_clock::duration delay,
    std::chrono::steady_clock::duration velocity_dt)
    : selected_cameras_{ selected_cameras }
    , delay_{ delay }
    , velocity_dt_{ velocity_dt }
{}

void AudioListenerUpdater::advance_time(float dt, const StaticWorld& world) {
    DanglingBaseClassRef<SceneNode> node = selected_cameras_.camera(CURRENT_SOURCE_LOCATION).node;
    auto corrected_time = std::chrono::steady_clock::now() - delay_;
    AudioScene::set_listener(node, corrected_time, velocity_dt_);
    AudioScene::flush_sources();
}
