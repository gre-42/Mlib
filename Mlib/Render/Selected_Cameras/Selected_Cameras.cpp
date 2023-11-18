#include "Selected_Cameras.hpp"
#include <Mlib/Render/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

SelectedCameras::SelectedCameras(Scene& scene)
    : camera_changed{ [this]() { return camera_node_exists(); } }
    , scene_{ scene }
    , dirtmap_node_name_{ "dirtmap_node" }
    , camera_cycle_near_{ *this, {"follower_camera", "turret_camera_node"} }  // "main_gun_end_node"
    , camera_cycle_far_{ *this, {"45_deg_camera", "light_node", "dirtmap_node"} }
    , camera_cycle_tripod_{ *this, {"tripod0"} }
    , fallback_camera_node_name_{ "stadium_camera" }
    , camera_node_name_{ "follower_camera" }
{}

SelectedCameras::~SelectedCameras()
{}

std::optional<CameraCycleType> SelectedCameras::cycle(const std::string& name) const {
    if (camera_cycle_near_.contains(name)) {
        return CameraCycleType::NEAR;
    }
    if (camera_cycle_far_.contains(name)) {
        return CameraCycleType::FAR;
    }
    if (camera_cycle_tripod_.contains(name)) {
        return CameraCycleType::TRIPOD;
    }
    return std::nullopt;
}

void SelectedCameras::set_camera_node_name(const std::string& name) {
    {
        std::scoped_lock lock{ camera_mutex_ };
        camera_node_name_ = name;
    }
    camera_changed.emit();
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

void SelectedCameras::set_camera_cycle(CameraCycleType tpe, const std::vector<std::string>& cameras) {
    if (tpe == CameraCycleType::NEAR) {
        camera_cycle_near_.set_camera_names(cameras);
    } else if (tpe == CameraCycleType::FAR) {
        camera_cycle_far_.set_camera_names(cameras);
    } else if (tpe == CameraCycleType::TRIPOD) {
        camera_cycle_tripod_.set_camera_names(cameras);
    } else {
        THROW_OR_ABORT("Unknown camera cycle type: " + std::to_string(int(tpe)));
    }
}

std::string SelectedCameras::dirtmap_node_name() const {
    return dirtmap_node_name_;
}

void SelectedCameras::cycle_camera(CameraCycleType tpe) {
    if (tpe == CameraCycleType::NEAR) {
        camera_cycle_near_.cycle_camera();
    } else if (tpe == CameraCycleType::FAR) {
        camera_cycle_far_.cycle_camera();
    } else if (tpe == CameraCycleType::TRIPOD) {
        camera_cycle_tripod_.cycle_camera();
    } else {
        THROW_OR_ABORT("Unknown camera cycle type: " + std::to_string(int(tpe)));
    }
}

bool SelectedCameras::camera_node_exists() const {
    return scene_.contains_node(camera_node_name());
}
