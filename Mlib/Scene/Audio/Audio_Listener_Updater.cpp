#include "Audio_Listener_Updater.hpp"
#include <Mlib/Audio/Audio_Listener.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

AudioListenerUpdater::AudioListenerUpdater(
    const SelectedCameras& selected_cameras,
    const Scene& scene)
: selected_cameras_{ selected_cameras },
  scene_{ scene }
{}

void AudioListenerUpdater::advance_time(float dt) {
    auto& node = scene_.get_node(selected_cameras_.camera_node_name());
    AudioListener::set_transformation(node.absolute_model_matrix());
}
