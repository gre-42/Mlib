#include "Audio_Listener_Position_Updater.hpp"
#include <Mlib/Audio/Audio_Listener.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

AudioListenerPositionUpdater::AudioListenerPositionUpdater(
    const SelectedCameras& selected_cameras,
    const Scene& scene)
: selected_cameras_{ selected_cameras },
  scene_{ scene }
{}

void AudioListenerPositionUpdater::advance_time(float dt) {
    const SceneNode* node = scene_.get_node(selected_cameras_.camera_node_name());
    AudioListener::set_transformation(node->absolute_model_matrix());
}
