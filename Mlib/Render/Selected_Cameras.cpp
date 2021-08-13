#include "Selected_Cameras.hpp"
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

SelectedCameras::SelectedCameras(Scene& scene)
: scene_{scene}
{}

void SelectedCameras::set_camera_node_name(const std::string& name) {
    camera_node_name_ = name;
}

const std::string& SelectedCameras::camera_node_name() const {
    if (scene_.contains_node(camera_node_name_)) {
        auto node = scene_.get_node(camera_node_name_);
        if (node->has_camera()) {
            return camera_node_name_;
        } else {
            return fallback_camera_node_name;
        }
    } else {
        return fallback_camera_node_name;
    }
}
