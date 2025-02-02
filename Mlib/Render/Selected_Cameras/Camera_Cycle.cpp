#include "Camera_Cycle.hpp"
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

CameraCycle::CameraCycle(
    SelectedCameras& selected_cameras,
    std::vector<std::string> camera_names)
    : selected_cameras_{ selected_cameras }
    , camera_names_{ std::move(camera_names) }
{}

CameraCycle::~CameraCycle() = default;

bool CameraCycle::contains(const std::string& name) const {
    std::shared_lock lock{ mutex_ };
    return std::find(camera_names_.begin(), camera_names_.end(), name) != camera_names_.end();
}

void CameraCycle::set_camera_names(const std::vector<std::string>& cameras) {
    std::scoped_lock lock{ mutex_ };
    camera_names_ = cameras;
}

void CameraCycle::cycle_camera() {
    std::shared_lock cycle_lock{ mutex_ };
    if (camera_names_.empty()) {
        // THROW_OR_ABORT("Camera cycle is empty");
        return;
    }
    auto it = std::find(camera_names_.begin(), camera_names_.end(), selected_cameras_.camera_node_name());
    if (it == camera_names_.end() || ++it == camera_names_.end()) {
        it = camera_names_.begin();
    }
    selected_cameras_.set_camera_node_name(*it);
}
