#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <chrono>

namespace Mlib {

class AudioListener;
class SelectedCameras;
class Scene;

class AudioListenerUpdater: public IAdvanceTime, public virtual DanglingBaseClass {
public:
    AudioListenerUpdater(
        const SelectedCameras& selected_cameras,
        const Scene& scene,
        std::chrono::steady_clock::duration delay,
        std::chrono::steady_clock::duration velocity_dt);
    virtual void advance_time(float dt, const StaticWorld& world) override;
private:
    const SelectedCameras& selected_cameras_;
    const Scene& scene_;
    std::chrono::steady_clock::duration delay_;
    std::chrono::steady_clock::duration velocity_dt_;
};

}
