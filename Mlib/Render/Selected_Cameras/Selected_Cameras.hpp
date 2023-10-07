#pragma once
#include <Mlib/Render/Selected_Cameras/Camera_Cycle.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

class Scene;
enum class CameraCycleType;

class SelectedCameras {
public:
    explicit SelectedCameras(Scene& scene);
    ~SelectedCameras();
    std::string camera_node_name() const;
    std::string dirtmap_node_name() const;
    void set_camera_node_name(const std::string& name);
    void set_camera_cycle(CameraCycleType tpe, const std::vector<std::string>& cameras);
    void cycle_camera(CameraCycleType tpe);
    std::optional<CameraCycleType> cycle(const std::string& name) const;
private:
    Scene& scene_;
    std::string dirtmap_node_name_;
    CameraCycle camera_cycle_near_;
    CameraCycle camera_cycle_far_;
    CameraCycle camera_cycle_tripod_;
    std::string fallback_camera_node_name_;
    std::string camera_node_name_;
    mutable SafeSharedMutex camera_mutex_;
};

}
