#pragma once
#include <Mlib/Scene_Graph/Interfaces/IDynamic_Lights.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <chrono>
#include <cstddef>
#include <functional>
#include <set>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class DynamicLightDb;
class IDynamicLight;

class DynamicLights: public IDynamicLights {
public:
    explicit DynamicLights(const DynamicLightDb& db);
    virtual ~DynamicLights() override;
    std::unique_ptr<IDynamicLight> instantiate(
        const std::string& name,
        const std::function<FixedArray<ScenePos, 3>()>& get_position,
        std::chrono::steady_clock::time_point time);
    void erase(IDynamicLight& light);

    // IDynamicLights
    virtual void append_time(std::chrono::steady_clock::time_point time) override;
    virtual void set_time(std::chrono::steady_clock::time_point time) override;
    virtual FixedArray<float, 3> get_color(const FixedArray<ScenePos, 3>& target_position) const override;

private:
    const DynamicLightDb& db_;
    std::set<IDynamicLight*> instances_;
    mutable AtomicMutex mutex_;
};

}
