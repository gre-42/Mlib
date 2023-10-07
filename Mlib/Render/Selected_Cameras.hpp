#pragma once
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <Mlib/Render/Camera_Cycle.hpp>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

class Scene;

class SelectedCameras {
public:
    explicit SelectedCameras(Scene& scene);
    ~SelectedCameras();
    void set_camera_node_name(const std::string& name);
    std::string camera_node_name() const;
    void set_camera_cycle_near(const std::vector<std::string>& cameras);
    void set_camera_cycle_far(const std::vector<std::string>& cameras);
    std::string dirtmap_node_name() const;
    void cycle_near_camera();
    void cycle_far_camera();
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
