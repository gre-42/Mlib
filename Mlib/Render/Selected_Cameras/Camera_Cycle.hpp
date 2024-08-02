#pragma once
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <string>
#include <vector>

namespace Mlib {

class SelectedCameras;

class CameraCycle {
public:
    CameraCycle(
        SelectedCameras& selected_cameras,
        std::vector<std::string> camera_names);
    ~CameraCycle();
    bool contains(const std::string& name) const;
    void set_camera_names(const std::vector<std::string>& cameras);
    void cycle_camera();
private:
    SelectedCameras& selected_cameras_;
    std::vector<std::string> camera_names_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
