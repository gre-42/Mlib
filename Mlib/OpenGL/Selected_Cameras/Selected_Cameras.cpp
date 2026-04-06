
#include "Selected_Cameras.hpp"
#include <Mlib/OpenGL/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <mutex>
#include <stdexcept>

using namespace Mlib;

using VH = VariableAndHash<std::string>;

SelectedCameras::SelectedCameras(Scene& scene)
    : camera_changed{ [this](const auto& f) { if (camera_node_exists()) { f(); }; } }
    , scene_{ scene }
    , dirtmap_node_name_{ "dirtmap_node" }
    , camera_cycle_near_{ *this, {VH{"follower_camera_0"}, VH{"turret_camera_node"}} }  // "main_gun_end_node"
    , camera_cycle_far_{ *this, {VH{"45_deg_camera"}, VH{"light_node"}, VH{"dirtmap_node"}} }
    , camera_cycle_spectator_{ *this, {VH{"spectator0"}} }
    , fallback_camera_node_name_{ "stadium_camera" }
    , camera_node_name_{ "follower_camera_0" }
{}

SelectedCameras::~SelectedCameras() = default;

std::optional<CameraCycleType> SelectedCameras::cycle(const VariableAndHash<std::string>& name) const {
    if (camera_cycle_near_.contains(name)) {
        return CameraCycleType::NEAR;
    }
    if (camera_cycle_far_.contains(name)) {
        return CameraCycleType::FAR;
    }
    if (camera_cycle_spectator_.contains(name)) {
        return CameraCycleType::SPECTATOR;
    }
    return std::nullopt;
}

void SelectedCameras::set_camera_node_name(const VariableAndHash<std::string>& name) {
    {
        std::scoped_lock lock{ camera_mutex_ };
        camera_node_name_ = name;
    }
    camera_changed.emit();
}

std::optional<NodeAndCamera> SelectedCameras::try_get_camera(const VariableAndHash<std::string>& name, SourceLocation loc) const {
    auto node = scene_.try_get_node(name, loc);
    if (node == nullptr) {
        return std::nullopt;
    }
    auto camera = node->try_get_camera(loc);
    if (camera == nullptr) {
        return std::nullopt;
    }
    return NodeAndCamera{
        .node = *node,
        .camera = *camera
    };
}

VariableAndHash<std::string> SelectedCameras::camera_node_name() const {
    VariableAndHash<std::string> cnn;
    {
        std::shared_lock lock{ camera_mutex_ };
        cnn = camera_node_name_;
    }
    auto res = try_get_camera(cnn, CURRENT_SOURCE_LOCATION);
    if (res.has_value()) {
        return cnn;
    }
    return fallback_camera_node_name_;
}

NodeAndCamera SelectedCameras::camera(SourceLocation loc) const {
    if (auto res = try_camera(loc); res.has_value()) {
        return *res;
    }
    throw std::runtime_error("Could not find camera");
}

std::optional<NodeAndCamera> SelectedCameras::try_camera(SourceLocation loc) const {
    VariableAndHash<std::string> cnn;
    {
        std::shared_lock lock{ camera_mutex_ };
        cnn = camera_node_name_;
    }
    if (auto res = try_get_camera(cnn, loc); res.has_value()) {
        return *res;
    }
    {
        std::shared_lock lock{ camera_mutex_ };
        cnn = fallback_camera_node_name_;
    }
    return try_get_camera(cnn, loc);
}

void SelectedCameras::set_camera_cycle(
    CameraCycleType tpe,
    const std::vector<VariableAndHash<std::string>>& cameras)
{
    switch (tpe) {
    case CameraCycleType::NEAR:
        camera_cycle_near_.set_camera_names(cameras);
        return;
    case CameraCycleType::FAR:
        camera_cycle_far_.set_camera_names(cameras);
        return;
    case CameraCycleType::SPECTATOR:
        camera_cycle_spectator_.set_camera_names(cameras);
        return;
    }
    throw std::runtime_error("Unknown camera cycle type: " + std::to_string(int(tpe)));
}

VariableAndHash<std::string> SelectedCameras::dirtmap_node_name() const {
    return dirtmap_node_name_;
}

void SelectedCameras::cycle_camera(CameraCycleType tpe) {
    switch (tpe) {
    case CameraCycleType::NEAR:
        camera_cycle_near_.cycle_camera();
        return;
    case CameraCycleType::FAR:
        camera_cycle_far_.cycle_camera();
        return;
    case CameraCycleType::SPECTATOR:
        camera_cycle_spectator_.cycle_camera();
        return;
    }
    throw std::runtime_error("Unknown camera cycle type: " + std::to_string(int(tpe)));
}

bool SelectedCameras::camera_node_exists() const {
    return try_camera(CURRENT_SOURCE_LOCATION).has_value();
}

void SelectedCameras::print_camera_cycles(std::ostream& ostr) const {
    ostr << "Selected camera: " << *camera_node_name() << '\n';
    ostr << "Near cycle:\n";
    camera_cycle_near_.print(ostr);
    ostr << "Far cycle:\n";
    camera_cycle_far_.print(ostr);
    ostr << "Spectator cycle:\n";
    camera_cycle_spectator_.print(ostr);
}
