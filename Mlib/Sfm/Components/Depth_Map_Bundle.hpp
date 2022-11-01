#pragma once
#include <chrono>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

template <class TMap>
class MarginalizedMap;
template <class TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
class ColoredVertexArray;

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
    void compute_roundtrip_error(const std::chrono::milliseconds& time, Array<float>& err, size_t& nerr) const;
    /**
     * From: Real-Time Visibility-Based Fusion of Depth Maps, ICCV 2007.
     */
    DepthMapBundle filtered(float eps_diff, const std::set<std::chrono::milliseconds>* references = nullptr) const;
    /**
     * From: https://github.com/cdcseacave/openMVS/blob/master/libs/MVS/SceneDensify.cpp
     *       void DepthMapsData::FuseDepthMaps(...)
     */
    DepthMapBundle delete_pixels_blocking_the_view(float threshold) const;
    DepthMapBundle reregistered(
        RegistrationDirection direction = RegistrationDirection::FORWARD,
        bool print_residual = false) const;
    Array<TransformationMatrix<float, float, 3>> points_and_normals(
        size_t k,
        float normal_radius,
        float duplicate_distance) const;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> mesh(
        const Array<TransformationMatrix<float, float, 3>>& point_cloud,
        float boundary_radius,
        float z_thickness,
        float cos_min_angle,
        float largest_cos_in_triangle) const;
private:
    Packages packages_;
};

}
