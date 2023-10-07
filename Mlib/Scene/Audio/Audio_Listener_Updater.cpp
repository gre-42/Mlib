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
    const Scene& scene)
#ifndef WITHOUT_ALUT
: selected_cameras_{ selected_cameras },
  scene_{ scene }
#endif
{}

void AudioListenerUpdater::advance_time(float dt) {
#ifndef WITHOUT_ALUT
    DanglingRef<SceneNode> node = scene_.get_node(selected_cameras_.camera_node_name(), DP_LOC);
    AudioListener::set_transformation(node->absolute_model_matrix());
#endif
}
