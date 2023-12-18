#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <chrono>

namespace Mlib {

class AudioListener;
class SelectedCameras;
class Scene;

class AudioListenerUpdater: public AdvanceTime {
public:
    AudioListenerUpdater(
        const SelectedCameras& selected_cameras,
        const Scene& scene,
        std::chrono::steady_clock::duration delay,
        std::chrono::steady_clock::duration velocity_dt);
    virtual void advance_time(float dt) override;
private:
#ifndef WITHOUT_ALUT
    const SelectedCameras& selected_cameras_;
    const Scene& scene_;
    std::chrono::steady_clock::duration delay_;
    std::chrono::steady_clock::duration velocity_dt_;
#endif
};

}
