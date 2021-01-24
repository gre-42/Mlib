#pragma once
#include <map>
#include <string>
#include <vector>

namespace Mlib {

class Scene;

class SelectedCameras {
public:
    explicit SelectedCameras(Scene& scene);
    std::string dirtmap_node_name = "dirtmap_node";
    std::vector<std::string> camera_cycle_near{"follower_camera", "turret_camera_node"};  // "main_gun_end_node";
    std::vector<std::string> camera_cycle_far{"45_deg_camera", "light_node", "dirtmap_node"};
    std::string fallback_camera_node_name = "light_node0";
    void set_camera_node_name(const std::string& name);
    const std::string& camera_node_name() const;
private:
    Scene& scene_;
    std::string camera_node_name_ = "follower_camera";
};

}
