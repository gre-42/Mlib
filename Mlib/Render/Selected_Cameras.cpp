#include "Selected_Cameras.hpp"
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

SelectedCameras::SelectedCameras(Scene& scene)
: scene_{scene}
{}

SelectedCameras::~SelectedCameras()
{}

void SelectedCameras::set_camera_node_name(const std::string& name) {
    std::scoped_lock lock{camera_mutex_};
    camera_node_name_ = name;
}

std::string SelectedCameras::camera_node_name() const {
    std::shared_lock lock{camera_mutex_};
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
    std::shared_lock lock{cycle_near_mutex_};
    return camera_cycle_near_;
}

std::vector<std::string> SelectedCameras::camera_cycle_far() const {
    std::shared_lock lock{cycle_far_mutex_};
    return camera_cycle_far_;
}

void SelectedCameras::set_camera_cycle_near(const std::vector<std::string>& cameras) {
    std::scoped_lock lock{cycle_near_mutex_};
    camera_cycle_near_ = cameras;
}

void SelectedCameras::set_camera_cycle_far(const std::vector<std::string>& cameras) {
    std::scoped_lock lock{cycle_far_mutex_};
    camera_cycle_far_ = cameras;
}

std::string SelectedCameras::dirtmap_node_name() const {
    return dirtmap_node_name_;
}

void SelectedCameras::cycle_near_camera() {
    std::shared_lock cycle_lock{cycle_near_mutex_};
    if (camera_cycle_near_.empty()) {
        THROW_OR_ABORT("Near camera cycle is empty");
    }
    auto it = std::find(camera_cycle_near_.begin(), camera_cycle_near_.end(), camera_node_name());
    if (it == camera_cycle_near_.end() || ++it == camera_cycle_near_.end()) {
        it = camera_cycle_near_.begin();
    }
    set_camera_node_name(*it);
}

void SelectedCameras::cycle_far_camera() {
    std::shared_lock cycle_lock{cycle_far_mutex_};
    if (camera_cycle_far_.empty()) {
        THROW_OR_ABORT("Far camera cycle is empty");
    }
    auto it = std::find(camera_cycle_far_.begin(), camera_cycle_far_.end(), camera_node_name());
    if (it == camera_cycle_far_.end() || ++it == camera_cycle_far_.end()) {
        it = camera_cycle_far_.begin();
    }
    set_camera_node_name(*it);
}
