#pragma once
#include <map>
#include <string>

namespace Mlib {

struct SelectedCameras {
    std::string camera_node_name = "follower_camera";
    std::string dirtmap_node_name = "dirtmap_node";
    std::vector<std::string> camera_cycle_near{"follower_camera", "turret-camera-node"};  // "main-gun-end-node";
    std::vector<std::string> camera_cycle_far{"45_deg_camera", "light_node", "dirtmap_node"};
};

}
