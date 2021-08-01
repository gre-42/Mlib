#pragma once
#include <chrono>
#include <map>

namespace Mlib {

template <class TMap>
class MarginalizedMap;
template <class TData>
class Array;

}

namespace Mlib::Sfm {

class CameraFrame;
class DownSampler;

class DepthMapBundle {
public:
    DepthMapBundle(
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const DownSampler& down_sampler);
    void insert(const std::chrono::milliseconds& time, const Array<float>& depth);
    void compute_error(const std::chrono::milliseconds& time, Array<float>& err, size_t& nerr) const;
private:
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
    std::map<std::chrono::milliseconds, Array<float>> depths_;
    const DownSampler& down_sampler_;
};

}
