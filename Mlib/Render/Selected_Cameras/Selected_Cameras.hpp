#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Event_Emitter.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

class Camera;
class SceneNode;
class Scene;
enum class CameraCycleType;

struct NodeAndCamera {
    DanglingBaseClassRef<SceneNode> node;
    DanglingBaseClassRef<Camera> camera;
};

class SelectedCameras {
    SelectedCameras(const SelectedCameras&) = delete;
    SelectedCameras& operator = (const SelectedCameras&) = delete;
public:
    explicit SelectedCameras(Scene& scene);
    ~SelectedCameras();
    NodeAndCamera camera(SOURCE_LOCATION loc) const;
    std::optional<NodeAndCamera> try_camera(SOURCE_LOCATION loc) const;
    bool camera_node_exists() const;
    VariableAndHash<std::string> camera_node_name() const;
    VariableAndHash<std::string> dirtmap_node_name() const;
    void set_camera_node_name(const VariableAndHash<std::string>& name);
    void set_camera_cycle(CameraCycleType tpe, const std::vector<VariableAndHash<std::string>>& cameras);
    void cycle_camera(CameraCycleType tpe);
    std::optional<CameraCycleType> cycle(const VariableAndHash<std::string>& name) const;
    EventEmitter<> camera_changed;
private:
    std::optional<NodeAndCamera> try_get_camera(const VariableAndHash<std::string>& name, SOURCE_LOCATION loc) const;
    Scene& scene_;
    VariableAndHash<std::string> dirtmap_node_name_;
    CameraCycle camera_cycle_near_;
    CameraCycle camera_cycle_far_;
    CameraCycle camera_cycle_tripod_;
    VariableAndHash<std::string> fallback_camera_node_name_;
    VariableAndHash<std::string> camera_node_name_;
    mutable SafeAtomicRecursiveSharedMutex camera_mutex_;
};

}
