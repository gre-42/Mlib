#pragma once
#include <map>
#include <shared_mutex>
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
    std::string impostor_camera_node_name() const;
private:
    std::string dirtmap_node_name_ = "dirtmap_node";
    std::vector<std::string> camera_cycle_near_{"follower_camera", "turret_camera_node"};  // "main_gun_end_node";
    std::vector<std::string> camera_cycle_far_{"45_deg_camera", "light_node", "dirtmap_node"};
    std::string fallback_camera_node_name_ = "stadium_camera";
    Scene& scene_;
    std::string camera_node_name_ = "follower_camera";
    std::string impostor_camera_node_ = "impostor_camera";
    mutable std::shared_mutex mutex_;
};

}
