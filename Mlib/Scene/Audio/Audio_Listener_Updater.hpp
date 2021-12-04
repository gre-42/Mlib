#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>

namespace Mlib {

class AudioListener;
class SelectedCameras;
class Scene;

class AudioListenerUpdater: public AdvanceTime {
public:
    AudioListenerUpdater(
        const SelectedCameras& selected_cameras,
        const Scene& scene);
    virtual void advance_time(float dt) override;
private:
    const SelectedCameras& selected_cameras_;
    const Scene& scene_;
};

}
