#pragma once
#include <map>
#include <string>

namespace Mlib {

struct SelectedCameras {
    std::string camera_node_name = "follower_camera";
    std::map<std::string, std::string> light_node_names;
    std::string dirtmap_node_name = "dirtmap_node";
    std::vector<std::string> camera_cycle_near{"follower_camera", "turret-camera-node"};  // "main-gun-end-node";
    std::vector<std::string> camera_cycle_far{"45_deg_camera", "light_node", "dirtmap_node"};
    inline std::string add_light_node(const std::string& name) {
        for(const auto& l : light_node_names) {
            if (l.second == name) {
                throw std::runtime_error("Duplicate light node " + name);
            }
        }
        std::string resource_id = name + "_light";
        if (!light_node_names.insert({resource_id, name}).second) {
            throw std::runtime_error("Duplicate light node identifier " + name);
        }
        return resource_id;
    }
};

}
