#include "Selected_Cameras.hpp"
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <mutex>

using namespace Mlib;

SelectedCameras::SelectedCameras(Scene& scene)
: scene_{scene}
{}

SelectedCameras::~SelectedCameras()
{}

void SelectedCameras::set_camera_node_name(const std::string& name) {
    std::unique_lock lock{mutex_};
    camera_node_name_ = name;
}

std::string SelectedCameras::camera_node_name() const {
    std::shared_lock lock{mutex_};
    if (scene_.contains_node(camera_node_name_)) {
        auto& node = scene_.get_node(camera_node_name_);
        if (node.has_camera()) {
            return camera_node_name_;
        } else {
            return fallback_camera_node_name_;
        }
    } else {
        return fallback_camera_node_name_;
    }
}

std::vector<std::string> SelectedCameras::camera_cycle_near() const {
    std::shared_lock lock{mutex_};
    return camera_cycle_near_;
}

std::vector<std::string> SelectedCameras::camera_cycle_far() const {
    std::shared_lock lock{mutex_};
    return camera_cycle_far_;
}

void SelectedCameras::set_camera_cycle_near(const std::vector<std::string>& cameras) {
    std::unique_lock lock{mutex_};
    camera_cycle_near_ = cameras;
}

void SelectedCameras::set_camera_cycle_far(const std::vector<std::string>& cameras) {
    std::unique_lock lock{mutex_};
    camera_cycle_far_ = cameras;
}

std::string SelectedCameras::dirtmap_node_name() const {
    std::shared_lock lock{mutex_};
    return dirtmap_node_name_;
}

std::string SelectedCameras::impostor_camera_node_name() const {
    std::shared_lock lock{mutex_};
    return impostor_camera_node_;
}
