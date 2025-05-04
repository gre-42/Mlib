#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <string>
#include <vector>

namespace Mlib {

template <class T>
class VariableAndHash;
class SelectedCameras;

class CameraCycle {
public:
    CameraCycle(
        SelectedCameras& selected_cameras,
        std::vector<VariableAndHash<std::string>> camera_names);
    ~CameraCycle();
    bool contains(const VariableAndHash<std::string>& name) const;
    void set_camera_names(std::vector<VariableAndHash<std::string>> cameras);
    void cycle_camera();
private:
    SelectedCameras& selected_cameras_;
    std::vector<VariableAndHash<std::string>> camera_names_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
