#pragma once
#include <chrono>
#include <map>

namespace Mlib {

template <class TMap>
class MarginalizedMap;
template <class TData>
class Array;

}

namespace Mlib::Cv {
    struct DepthMapPackage;
}

namespace Mlib::Sfm {

class CameraFrame;
class DownSampler;
enum class RegistrationDirection {
    FORWARD,
    BACKWARD
};

class DepthMapBundle {
public:
    typedef std::map<std::chrono::milliseconds, Cv::DepthMapPackage> Packages;
    DepthMapBundle();
    ~DepthMapBundle();
    void insert(const Cv::DepthMapPackage& package);
    const Packages& packages() const;
    void compute_error(const std::chrono::milliseconds& time, Array<float>& err, size_t& nerr) const;
    DepthMapBundle delete_pixels_blocking_the_view(float threshold) const;
    DepthMapBundle reregister(
        RegistrationDirection direction = RegistrationDirection::FORWARD,
        bool print_residual = false) const;
private:
    Packages packages_;
};

}
