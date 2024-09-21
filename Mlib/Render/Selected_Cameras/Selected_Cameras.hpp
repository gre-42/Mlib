#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Event_Emitter.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

class Camera;
class SceneNode;
class Scene;
enum class CameraCycleType;

class SelectedCameras {
    SelectedCameras(const SelectedCameras&) = delete;
    SelectedCameras& operator = (const SelectedCameras&) = delete;
public:
    explicit SelectedCameras(Scene& scene);
    ~SelectedCameras();
    DanglingBaseClassRef<Camera> camera() const;
    DanglingRef<SceneNode> camera_node() const;
    DanglingPtr<SceneNode> try_camera_node() const;
    bool camera_node_exists() const;
    std::string camera_node_name() const;
    std::string dirtmap_node_name() const;
    void set_camera_node_name(const std::string& name);
    void set_camera_cycle(CameraCycleType tpe, const std::vector<std::string>& cameras);
    void cycle_camera(CameraCycleType tpe);
    std::optional<CameraCycleType> cycle(const std::string& name) const;
    EventEmitter camera_changed;
private:
    DanglingBaseClassPtr<Camera> try_get_camera(const std::string& name) const;
    DanglingPtr<SceneNode> try_get_camera_node(const std::string& name) const;
    Scene& scene_;
    std::string dirtmap_node_name_;
    CameraCycle camera_cycle_near_;
    CameraCycle camera_cycle_far_;
    CameraCycle camera_cycle_tripod_;
    std::string fallback_camera_node_name_;
    std::string camera_node_name_;
    mutable SafeAtomicRecursiveSharedMutex camera_mutex_;
};

}
