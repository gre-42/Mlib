#include "Selected_Cameras.hpp"
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

SelectedCameras::SelectedCameras(Scene& scene)
: scene_{scene},
  dirtmap_node_name_{ "dirtmap_node" },
  camera_cycle_near_{ *this, {"follower_camera", "turret_camera_node"} },  // "main_gun_end_node"
  camera_cycle_far_{ *this, {"45_deg_camera", "light_node", "dirtmap_node"} },
  camera_cycle_tripod_{ *this, {"tripod0"} },
  fallback_camera_node_name_{ "stadium_camera" },
  camera_node_name_{ "follower_camera" }
{}

SelectedCameras::~SelectedCameras()
{}

void SelectedCameras::set_camera_node_name(const std::string& name) {
    std::scoped_lock lock{camera_mutex_};
    camera_node_name_ = name;
}

std::string SelectedCameras::camera_node_name() const {
    std::string cnn;
    {
        std::shared_lock lock{camera_mutex_};
        cnn = camera_node_name_;
    }
    if (scene_.contains_node(cnn)) {
        DanglingRef<SceneNode> node = scene_.get_node(cnn, DP_LOC);
        if (node->has_camera()) {
            return camera_node_name_;
        } else {
            std::shared_lock lock{camera_mutex_};
            return fallback_camera_node_name_;
        }
    } else {
        std::shared_lock lock{camera_mutex_};
        return fallback_camera_node_name_;
    }
}

void SelectedCameras::set_camera_cycle_near(const std::vector<std::string>& cameras) {
    camera_cycle_near_.set_camera_names(cameras);
}

void SelectedCameras::set_camera_cycle_far(const std::vector<std::string>& cameras) {
    camera_cycle_far_.set_camera_names(cameras);
}

std::string SelectedCameras::dirtmap_node_name() const {
    return dirtmap_node_name_;
}

void SelectedCameras::cycle_near_camera() {
    camera_cycle_near_.cycle_camera();
}

void SelectedCameras::cycle_far_camera() {
    camera_cycle_far_.cycle_camera();
}
