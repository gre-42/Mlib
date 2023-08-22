#pragma once
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
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
    std::vector<std::string> camera_cycle_near() const;
    std::vector<std::string> camera_cycle_far() const;
    void set_camera_cycle_near(const std::vector<std::string>& cameras);
    void set_camera_cycle_far(const std::vector<std::string>& cameras);
    std::string dirtmap_node_name() const;
    void cycle_near_camera();
    void cycle_far_camera();
private:
    std::string dirtmap_node_name_ = "dirtmap_node";
    std::vector<std::string> camera_cycle_near_{"follower_camera", "turret_camera_node"};  // "main_gun_end_node";
    std::vector<std::string> camera_cycle_far_{"45_deg_camera", "light_node", "dirtmap_node"};
    std::string fallback_camera_node_name_ = "stadium_camera";
    Scene& scene_;
    std::string camera_node_name_ = "follower_camera";
    mutable SafeSharedMutex cycle_near_mutex_;
    mutable SafeSharedMutex cycle_far_mutex_;
    mutable SafeSharedMutex camera_mutex_;
};

}
