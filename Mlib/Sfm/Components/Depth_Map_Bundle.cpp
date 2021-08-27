#include "Depth_Map_Bundle.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Depth_Minus.hpp>
#include <Mlib/Cv/Project_Depth_Map.hpp>
#include <Mlib/Cv/Rigid_Motion/Rigid_Motion_Roundtrip.hpp>
#include <Mlib/Cv/Matrix_Conversion.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Look_At.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images_Smooth.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <Mlib/Stats/Sort.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

class DepthMapPackageWindow {
public:
    DepthMapPackageWindow(
        const DepthMapBundle::Packages& packages,
        const std::chrono::milliseconds& time,
        size_t max_distance)
    : packages_{packages},
      time_{time},
      max_distance_{max_distance}
    {
        cit = packages_.find(time);
        if (cit == packages_.end()) {
            throw std::runtime_error("Could not find depth at time " + std::to_string(time.count()) + " ms");
        }
    }
    DepthMapBundle::Packages::const_iterator begin() const {
        auto bit = cit;
        for (size_t i = 0; bit != packages_.begin() && i < max_distance_; --bit, ++i);
        return bit;
    }
    DepthMapBundle::Packages::const_iterator end() const {
        auto eit = cit;
        for (size_t i = 0; eit != packages_.end() && i < max_distance_; ++eit, ++i);
        return eit;
    }
private:
    DepthMapBundle::Packages::const_iterator cit;
    const DepthMapBundle::Packages& packages_;
    std::chrono::milliseconds time_;
    size_t max_distance_;
};

DepthMapBundle::DepthMapBundle()
{}

DepthMapBundle::~DepthMapBundle()
{}

void DepthMapBundle::insert(const DepthMapPackage& package) {
    if (!packages_.insert({ package.time, package }).second) {
        throw std::runtime_error("Depth at time " + std::to_string(package.time.count()) + " already exists");
    }
}

const DepthMapBundle::Packages& DepthMapBundle::packages() const {
    return packages_;
}

void DepthMapBundle::compute_roundtrip_error(const std::chrono::milliseconds& time, Array<float>& err, size_t& nerr) const {
    size_t max_distance = 2;
    DepthMapPackageWindow window{ packages_, time, max_distance };
    
    const DepthMapPackage& c = packages_.at(time);

    err = zeros<float>(c.depth.shape());
    nerr = 0;
    for (const auto& neighbor : window) {
        if (neighbor.first == time) {
            continue;
        }
        std::cerr << "Keyframe " << time.count() <<
            " ms selected neighbor " << neighbor.first.count() << " ms" << std::endl;
        // ke is expected to be l's relative projection-matrix.
        TransformationMatrix<float, 3> ke = projection_in_reference(
            c.ke,
            neighbor.second.ke);
        err += rigid_motion_roundtrip(
            c.depth,
            neighbor.second.depth,
            c.ki,
            neighbor.second.ki,
            ke);
        // draw_nan_masked_grayscale(err, 0, 0.5 * 0.5).save_to_file(cache_dir_ + "/err-" + suffix + "-" + std::to_string(neighbor->first.count()) + ".png");
        ++nerr;
    }
}

DepthMapBundle DepthMapBundle::filtered(
    float eps_diff,
    const std::set<std::chrono::milliseconds>* references) const
{
    DepthMapBundle result;
    for (const auto& reference : packages_) {
        if ((references != nullptr) && !references->contains(reference.first)) {
            continue;
        }
        std::cerr << "Filtering time " << reference.second.time.count() << " ms" << std::endl;

        // 1. Who occludes the reference?
        Array<float> occluder_depths{ ArrayShape{ packages_.size() }.concatenated(reference.second.depth.shape()) };
        {
            size_t i = 0;
            for (const auto& frame_i : packages_) {
                if (reference.second.time == frame_i.second.time) {
                    occluder_depths[i] = frame_i.second.depth;
                } else {
                    Array<float> occluder_rgb;
                    Array<float> occluder_depth(occluder_depths[i]);
                    project_depth_map(
                        frame_i.second.rgb,
                        frame_i.second.depth,
                        frame_i.second.ki,
                        projection_in_reference(frame_i.second.ke, reference.second.ke),
                        occluder_rgb,
                        occluder_depth,
                        reference.second.ki,
                        reference.second.depth.shape(1),
                        reference.second.depth.shape(0),
                        0.1f,       // z_near
                        100.f);     // z_far
                }
                ++i;
            }
        }
        occluder_depths.move() = nan_sorted(occluder_depths, 0);

        // 2. Who does the reference occlude (a.k.a. depth violations)?
        Array<size_t> depth_index = zeros<size_t>(reference.second.depth.shape());
        Array<float> filtered_reference_depth = occluder_depths[0].copy();
        for (size_t n = 0; n < packages_.size(); ++n) {
            Array<size_t> n_depth_violations = zeros<size_t>(reference.second.depth.shape());
            for (const auto& frame_i : packages_) {
                Array<float> depth_violation_diff;
                if (reference.second.time == frame_i.second.time) {
                    depth_violation_diff.move() = filtered_reference_depth - frame_i.second.depth;
                } else {
                    depth_violation_diff.move() = Cv::depth_minus(
                        filtered_reference_depth,
                        frame_i.second.depth,
                        reference.second.ki,
                        frame_i.second.ki,
                        projection_in_reference(reference.second.ke, frame_i.second.ke));
                }
                // Violation if: filtered_reference_depth < frame_i_depth => filtered_reference_depth - frame_i_depth < 0
                n_depth_violations += (substitute_nans(depth_violation_diff, INFINITY) < -eps_diff).casted<size_t>();
            }
            depth_index.move() = depth_index.array_array_binop(n_depth_violations, [](size_t depth_id, size_t n_violations){
                return n_violations > depth_id + 1
                    ? depth_id + 1
                    : depth_id; });
            for (size_t r = 0; r < filtered_reference_depth.shape(0); ++r) {
                for (size_t c = 0; c < filtered_reference_depth.shape(1); ++c) {
                    filtered_reference_depth(r, c) = occluder_depths(depth_index(r, c), r, c);
                }
            }
        }
        result.insert(DepthMapPackage{
            .time = reference.second.time,
            .rgb = reference.second.rgb,
            .depth = filtered_reference_depth,
            .ki = reference.second.ki,
            .ke = reference.second.ke});
    }
    return result;
}

DepthMapBundle DepthMapBundle::delete_pixels_blocking_the_view(float threshold) const {
    DepthMapBundle result;
    for (const auto& plus : packages_) {
        Array<float> depth = plus.second.depth.copy();
        for (const auto& minus : packages_) {
            if (plus.second.time == minus.second.time) {
                continue;
            }
            Array<float> diff = Cv::depth_minus(
                depth,
                minus.second.depth,
                plus.second.ki,
                minus.second.ki,
                projection_in_reference(plus.second.ke, minus.second.ke));
            // NAN if: plus_depth < minus_depth => plus_depth - minus_depth < 0
            depth.move() = depth.array_array_binop(diff, [&threshold](float a, float b){ return std::isnan(b) || (b < -threshold) ? NAN : a; });
        }
        result.insert(DepthMapPackage{
            .time = plus.second.time,
            .rgb = plus.second.rgb,
            .depth = depth,
            .ki = plus.second.ki,
            .ke = plus.second.ke});
    }
    return result;
}

DepthMapBundle DepthMapBundle::reregistered(
    RegistrationDirection direction,
    bool print_residual) const
{
    auto reg = [print_residual](auto begin, auto end){
        DepthMapBundle result;
        auto eit = begin;
        if (eit == end) {
            return result;
        }
        result.insert(eit->second);
        while (true) {
            auto bit = eit++;
            if (eit == end) {
                break;
            }
            std::cerr << "Reregistering times " << bit->second.time.count() << " ms and " << eit->second.time.count() << " ms" << std::endl;
            TransformationMatrix<float, 3> x0_r1_r0 = projection_in_reference(
                bit->second.ke,
                eit->second.ke);
            TransformationMatrix<float, 3> ke = Rmfi::rigid_motion_from_images_smooth(
                bit->second.rgb,
                eit->second.rgb,
                bit->second.depth,
                bit->second.ki,
                eit->second.ki,
                {3.f, 1.f, 0.f},
                k_external_inverse(x0_r1_r0),
                print_residual);
            result.insert(DepthMapPackage{
                .time = eit->second.time,
                .rgb = eit->second.rgb,
                .depth = eit->second.depth,
                .ki = eit->second.ki,
                .ke = ke * result.packages_.at(bit->first).ke});
        }
        return result;
    };
    if (direction == RegistrationDirection::FORWARD) {
        return reg(packages_.begin(), packages_.end());
    } else {
        return reg(packages_.rbegin(), packages_.rend());
    }
}

Array<TransformationMatrix<float, 3>> DepthMapBundle::points_and_normals(size_t k, float normal_radius) const
{
    Array<FixedArray<float, 3>> points;
    Array<FixedArray<float, 3>> normals;
    Array<FixedArray<float, 3>> dys;

    points.resize(ArrayShape{ 0 });
    dys.resize(ArrayShape{ 0 });
    Array<FixedArray<float, 3>> zs(ArrayShape{ 0 });
    for (const auto& package : packages_) {
        TransformationMatrix cpos = package.second.ke.inverted_scaled();
        for (size_t r = 0; r < package.second.depth.shape(0); ++r) {
            for (size_t c = 0; c < package.second.depth.shape(1); ++c) {
                if (std::isnan(package.second.depth(r, c))) {
                    continue;
                }
                TransformationMatrix<float, 2> iim{ inv(package.second.ki.affine()) };
                FixedArray<size_t, 2> id{r, c};
                FixedArray<float, 2> lifted = iim.transform(i2a(id));
                FixedArray<float, 3> pos0{
                    lifted(0) * package.second.depth(r, c),
                    lifted(1) * package.second.depth(r, c),
                    package.second.depth(r, c)};
                points.append(cpos.transform(pos0));
                dys.append(package.second.ke.R()[1]);
                FixedArray<float, 3> z{ cpos.rotate(pos0) };
                zs.append(z / std::sqrt(sum(squared(z))));
            }
        }
    }
    Bvh<float, FixedArray<float, 3>, 3> bvh{{0.1f, 0.1f, 0.1f}, 10};
    for (auto& p : points.flat_iterable()) {
        bvh.insert({p}, p);
    }
    normals.resize(ArrayShape{ 0 });
    for (size_t pi = 0; pi < points.length(); ++pi) {
        const auto& p = points(pi);
        const auto& z = zs(pi);
        std::vector<std::pair<float, const FixedArray<float, 3>*>> k_nearest = bvh.min_distances(
            k,
            p,
            normal_radius,
            [&p](const FixedArray<float, 3>& a){return sum(squared(a - p));});
        FixedArray<float, 3> normal{ 0.f, 0.f, 0.f };
        size_t nnormals = 0;
        for (size_t i = 0; i < k_nearest.size(); ++i) {
            for (size_t j = i + 1; j < k_nearest.size(); ++j) {
                FixedArray<float, 3> n = cross(p - *k_nearest[i].second, p - *k_nearest[j].second);
                float len2 = sum(squared(n));
                if (len2 < 1e-12) {
                    continue;
                }
                if (dot0d(n, z) < 0.f) {
                    n = -n;
                }
                normal += n / std::sqrt(len2);
                ++nnormals;
            }
        }
        float len2 = sum(squared(normal));
        if (len2 < 1e-12) {
            normals.append(fixed_nans<float, 3>());
        } else {
            normals.append(normal / float(nnormals));
        }
    }

    Array<TransformationMatrix<float, 3>> result{ ArrayShape{ 0 } };
    for (size_t i = 0; i < points.length(); ++i) {
        const FixedArray<float, 3>& normal = normals(i);
        if (all(isnan(normal))) {
            continue;
        }
        result.append(
            opengl_matrix_from_opencv_extrinsic_matrix(
                TransformationMatrix<float, 3>{
                lookat_relative(
                    cv_to_opengl_coordinates(normals(i)),
                    cv_to_opengl_coordinates(dys(i))),
                cv_to_opengl_coordinates(points(i)) }));
    }
    return result;
}
