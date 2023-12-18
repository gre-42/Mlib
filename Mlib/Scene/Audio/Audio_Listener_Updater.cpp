#include "Audio_Listener_Updater.hpp"
#ifndef WITHOUT_ALUT
#include <Mlib/Audio/Audio_Listener.hpp>
#endif
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

AudioListenerUpdater::AudioListenerUpdater(
    const SelectedCameras& selected_cameras,
    const Scene& scene,
    std::chrono::steady_clock::duration delay,
    std::chrono::steady_clock::duration velocity_dt)
#ifndef WITHOUT_ALUT
    : selected_cameras_{ selected_cameras }
    , scene_{ scene }
    , delay_{ delay }
    , velocity_dt_{ velocity_dt }
#endif
{}

void AudioListenerUpdater::advance_time(float dt) {
#ifndef WITHOUT_ALUT
    DanglingRef<SceneNode> node = scene_.get_node(selected_cameras_.camera_node_name(), DP_LOC);
    auto time = std::chrono::steady_clock::now() - delay_;
    AudioListener::set_transformation(AudioListenerState{
        .pose = node->absolute_model_matrix(time),
        .velocity = node->velocity(time, velocity_dt_)
    });
#endif
}
