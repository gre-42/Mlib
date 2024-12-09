#include "Flowing_Particles.hpp"
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/CvSift/CvSift.hpp>
#include <Mlib/Images/CvSift/CvSift2.hpp>
#include <Mlib/Images/CvSift/KeyPoint.hpp>
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Box_Filter.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/OpenCV.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Configuration/Tracking_Mode.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Descriptor.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <filesystem>

#ifndef WITHOUT_OPENCV
#include <opencv2/features2d.hpp>
#endif

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::Sfm;

FlowingParticles::FlowingParticles(
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
    const std::map<std::chrono::milliseconds, ImageFrame>& optical_flow_frames,
    const std::string& cache_dir,
    const FlowingParticlesConfig& cfg)
: image_frames_{image_frames},
  optical_flow_frames_{optical_flow_frames},
  shape_{SIZE_MAX},
  cache_dir_{cache_dir},
  cfg_{cfg}
{}

void FlowingParticles::generate_sift_correspondences(FeaturePointFrame& new_frame) {
    assert(image_frames_.size() > 0);
    shape_ = image_frames_.rbegin()->second.grayscale.fixed_shape<2>();

    Array<FixedArray<float, 2>>& keypoints1 = new_frame.keypoints;
    Array<float>& descriptors1 = new_frame.descriptors;
    Array<uint8_t> grayscale_u8 = image_frames_.rbegin()->second.grayscale.applied<uint8_t>([](float v){return (uint8_t)(std::min(v, 1.f) * 255);});
    if (cfg_.tracking_mode == TrackingMode::CV_SIFT_0) {
        std::vector<ocv::KeyPoint> keypoints1_;
        ocv::SIFT sift{ (int)cfg_.target_nparticles };
        sift(grayscale_u8,
            ones<uint8_t>(grayscale_u8.shape()),
            keypoints1_,
            &descriptors1);
        keypoints1.resize(ArrayShape{keypoints1_.size()});
        for (size_t i = 0; i < keypoints1_.size(); ++i) {
            keypoints1(i) = keypoints1_[i].pt;
        }
    } else if (cfg_.tracking_mode == TrackingMode::CV_SIFT_1) {
        std::vector<ocv::KeyPoint> keypoints1_;
        ocv::SIFT2 sift{ (int)cfg_.target_nparticles };
        sift.detectAndCompute(grayscale_u8,
            ones<uint8_t>(grayscale_u8.shape()),
            keypoints1_,
            &descriptors1);
        keypoints1.resize(ArrayShape{keypoints1_.size()});
        for (size_t i = 0; i < keypoints1_.size(); ++i) {
            keypoints1(i) = keypoints1_[i].pt;
        }
    } else if (cfg_.tracking_mode == TrackingMode::CV_SIFT) {
#ifndef WITHOUT_OPENCV
        auto sift = cv::SIFT::create((int)cfg_.target_nparticles);
        std::vector<cv::KeyPoint> cv_keypoints;
        cv::Mat_<float> cv_descriptors1;
        sift->detectAndCompute(
            array_to_cv_mat(grayscale_u8),
            cv::Mat_<uint8_t>(),
            cv_keypoints,
            cv_descriptors1);
        descriptors1 = cv_mat_to_array(cv_descriptors1);
        keypoints1.resize(ArrayShape{cv_keypoints.size()});
        for (size_t i = 0; i < cv_keypoints.size(); ++i) {
            keypoints1(i) = FixedArray<float, 2>{ cv_keypoints[i].pt.x, cv_keypoints[i].pt.y };
        }
#else
        throw std::runtime_error("Compiled without OpenCV");
#endif
    } else {
        throw std::runtime_error("Unknown tracking mode");
    }
    std::map<size_t, size_t> inserted_keypoints1;
    std::list<std::pair<size_t, size_t>> matches;
    if (particles_.size() >= 1) {
        auto pit = particles_.begin();
        lerr() << "SIFT: " << pit->first.count() << " <-> " << image_frames_.rbegin()->first.count() << " ms";
        for (auto& s : pit->second.tracked_points) {
            size_t best_id1 = s.second->sequence.begin()->second->tracebale_descriptor.descriptor_id_in_parameter_list(descriptors1);
            if ((best_id1 != SIZE_MAX) && !inserted_keypoints1.contains(best_id1)) {
                matches.push_back({ s.first, best_id1 });
            }
            ++inserted_keypoints1[best_id1];
        }
        for (const auto& m : matches) {
            if (inserted_keypoints1.at(m.second) == 1) {
                try_insert_and_append_feature_point(
                    new_frame,
                    *pit->second.tracked_points.find(m.first),
                    keypoints1(m.second),
                    descriptors1[m.second]);
            }
        }
    }
    for (size_t i1 = 0; (i1 < keypoints1.length()) && (new_frame.tracked_points.size() < cfg_.target_nparticles); ++i1) {
        if (!inserted_keypoints1.contains(i1)) {
            try_generate_feature_point_sequence(
                new_frame,
                keypoints1(i1),
                descriptors1[i1]);
        }
    }
}

void FlowingParticles::generate_new_particles(FeaturePointFrame& new_frame) {
    assert(image_frames_.size() > 0);
    shape_ = image_frames_.rbegin()->second.grayscale.fixed_shape<2>();
    assert(new_frame.tracked_points.size() <= cfg_.target_nparticles);
    const size_t n_new_particles = cfg_.target_nparticles - new_frame.tracked_points.size();

    if (false) {
        for (size_t r = 0; r < shape_(id0); r+=shape_(id0)/10) {
            for (size_t c = 0; c < shape_(id1); c+=shape_(id1)/10) {
                try_generate_feature_point_sequence(
                    new_frame,
                    i2a(FixedArray<size_t, 2>{r, c}),
                    Array<float>());
            }
        }
    }

    Array<bool> existing_points_mask = ones<bool>(image_frames_.rbegin()->second.grayscale.shape());
    for (const auto& kv : new_frame.tracked_points) {
        draw_fill_rect(
            existing_points_mask,
            a2i(kv.second->sequence.rbegin()->second->position),
            5,      //size
            false); //value
    }

    Array<FixedArray<float, 2>> points;
    if (false) {
        Array<float> h_trace;
        Array<float> h_det;
        float r_thr = 10;
        hessian_determinant_trace(box_filter_NWE(image_frames_.rbegin()->second.grayscale, ArrayShape{10, 10}), &h_det, &h_trace);
        Array<float> R = squared(h_trace) / (h_det + float(1e-6));
        points = Array<float>::from_dynamic<2>(find_nfeatures(harris_response(
            image_frames_.rbegin()->second.grayscale),
            existing_points_mask && (R < squared(r_thr + 1) / r_thr),
            n_new_particles));
        //Bgr565Bitmap::from_float_grayscale(
        //    substitute_nans(
        //        normalized_and_clipped(
        //            R, 0.f, squared(r_thr + 1) / r_thr),
        //        0.f))
        //    .save_to_file("edges.bmp");
    } else if (true) {
        Array<float> hr = harris_response(
            gaussian_filter_NWE(
                image_frames_.rbegin()->second.grayscale,
                cfg_.presmoothing_sigma, NAN));
        points = Array<float>::from_dynamic<2>(find_nfeatures(
            hr,

            // normalize_brightness(
            //     -hr,
            //     ArrayShape{100, 100},
            //     0.f),

            // -hr /
            // (box_filter(image_frames_.rbegin()->second.grayscale, ArrayShape{3, 3}, NAN) + 10.f),

            // abs(laplace_filter(image_frames_.rbegin()->second.grayscale, NAN)),

            existing_points_mask,
            n_new_particles,
            cfg_.distance_sigma));
    } else if (false) {
        Array<bool> mask = multi_scale_harris(image_frames_.rbegin()->second.grayscale, 5);
        points = Array<float>::from_dynamic<2>(find_nfeatures(
            harris_response(image_frames_.rbegin()->second.grayscale),
            existing_points_mask && mask,
            n_new_particles));
        //lerr() << points.shape();
        //lerr() << count_nonzero(mask);
    }
    lerr() << "Trying to generate " << points.shape(0) << " new particles";
    for (const FixedArray<float, 2>& p : points.flat_iterable()) {
        try_generate_feature_point_sequence(new_frame, p, Array<float>());
    }
}

size_t FlowingParticles::streamline_search_length() {
    assert(image_frames_.size() > 0);
    return (5 * max(Array<size_t>::from_shape(
        image_frames_.rbegin()->second.grayscale.shape()))) / 113;
}

std::shared_ptr<FeaturePoint> FlowingParticles::generate_feature_point(
    const FixedArray<float, 2>& pos,
    const Array<float>& descriptor) const
{
    return std::make_shared<FeaturePoint>(
        pos,
        TraceablePatch(
            image_frames_.rbegin()->second.rgb,
            a2i(pos),
            cfg_.patch_size),
        TraceableDescriptor{ descriptor });
}

bool FlowingParticles::try_generate_feature_point_sequence(
    FeaturePointFrame& frame,
    const FixedArray<float, 2>& pos,
    const Array<float>& descriptor)
{
    auto p = generate_feature_point(pos, descriptor);
    if (p->traceable_patch.good_) {
        auto seq = std::make_shared<FeaturePointSequence>();
        seq->sequence[image_frames_.rbegin()->first] = p;
        frame.tracked_points[sequence_index++] = seq;
        return true;
    }
    return false;
}

bool FlowingParticles::try_insert_and_append_feature_point(
    FeaturePointFrame& frame,
    const std::pair<size_t, std::shared_ptr<FeaturePointSequence>>& seq,
    const FixedArray<float, 2>& pos,
    const Array<float>& descriptor)
{
    auto p = generate_feature_point(pos, descriptor);
    if (descriptor.initialized() || p->traceable_patch.good_) {
        seq.second->sequence[image_frames_.rbegin()->first] = p;
        if (!frame.tracked_points.insert(seq).second) {
            throw std::runtime_error("Point to be inserted already exists");
        }
        return true;
    }
    return false;
}

void FlowingParticles::advance_existing_particles(
    FeaturePointFrame& new_frame,
    const Array<float>& flow_field,
    const Array<bool>& flow_mask)
{
    assert(image_frames_.size() > 0);
    Array<float> hr = harris_response(image_frames_.rbegin()->second.grayscale);
    /*assert(image_frames_.size() > 1);
    Array<float> displacement;
    flow_registration(
        (image_frames_.end() - 1)->image,
        (image_frames_.end() - 2)->image,
        displacement,
        3,
        2,
        5,
        10);*/
    /*Array<float> fff(flow_field.shape());
    for (size_t d = 0; d < flow_field.shape(0); ++d) {
        fff[d] = box_filter_nans_as_zeros_NWE(flow_field[d], ArrayShape{3, 3});
    }*/
    assert(particles_.size() > 0);
    if (requires_optical_flow()) {
        assert(all(flow_field.shape().erased_first() == ArrayShape{ shape_(0), shape_(1) }));
        assert(flow_field.shape(0) == 2);
    }
    size_t nsuccess = 0;
    for (const auto& s : particles_.rbegin()->second.tracked_points) {
        //if (bad_points_.find(s.first) != bad_points_.end()) {
        //    lerr() << "Discontinuing bad point [" << s.first << "]";
        //    continue;
        //}
        const FeaturePoint& p = *s.second->sequence.rbegin()->second;
        FixedArray<size_t, 2> index{a2i(p.position)};
        if (all(index < shape_)) {
            std::optional<FixedArray<float, 2>> new_pos;
            if (false) {
                if (!flow_mask(index)) {
                    continue;
                }
                FixedArray<float, 2> v{
                    flow_field[0](index),
                    flow_field[1](index)};
                new_pos = find_feature_in_neighborhood(p.position + v, hr, ArrayShape{5, 5});
            } else if (false) {
                if (!flow_mask(index)) {
                    continue;
                }
                float best_harris = -std::numeric_limits<float>::infinity();
                visit_streamline(shape_, index, flow_field, streamline_search_length(), [&](const FixedArray<size_t, 2>& id) {
                    if (hr(id) > best_harris) {
                        best_harris = hr(id);
                        new_pos = FixedArray<float, 2>{ i2a(id) };
                    }
                });
            }
            if (cfg_.tracking_mode == TrackingMode::PATCH_NEW_POSITION_IN_BOX) {
                FixedArray<size_t, 2> new_index = s.second->sequence.begin()->second->traceable_patch.new_position_in_box(
                    image_frames_.rbegin()->second.rgb,
                    a2i(p.position),
                    cfg_.search_window,
                    cfg_.worst_patch_error);
                new_pos = FixedArray<float, 2>{ i2a(new_index) };
                if (any(*new_pos > float(1e6))) {
                    // lerr() << "dropping y [" << s.first << "]";
                    continue;
                }
            }
            if (try_insert_and_append_feature_point(new_frame, s, new_pos.value(), Array<float>())) {
                ++nsuccess;
            }
        }
    }
    lerr() << "Traced " << nsuccess << " / " << particles_.rbegin()->second.tracked_points.size() << " particles";
}

void FlowingParticles::advance_flowing_particles() {
    assert(image_frames_.size() >= 1);
    FeaturePointFrame new_frame;
    new_frame.tracking_mode = cfg_.tracking_mode;
    if (cfg_.tracking_mode & TrackingMode::SIFT) {
        generate_sift_correspondences(new_frame);
    } else {
        if (particles_.size() > 0) {
            if (cfg_.tracking_mode == TrackingMode::PATCH_NEW_POSITION_IN_BOX) {
                advance_existing_particles(
                    new_frame,
                    Array<float>(),
                    Array<bool>());
            } else {
                assert(optical_flow_frames_.size() >= 1);
                advance_existing_particles(
                    new_frame,
                    optical_flow_frames_.rbegin()->second.grayscale,
                    optical_flow_frames_.rbegin()->second.mask);
            }
        }
        generate_new_particles(new_frame);
    }
    particles_.insert(std::make_pair(image_frames_.rbegin()->first, new_frame));
    StbImage3 bmp = StbImage3::from_float_rgb(image_frames_.rbegin()->second.rgb);
    if (optical_flow_frames_.size() >= 1) {
        bmp.draw_mask(optical_flow_frames_.rbegin()->second.mask, Rgb24::red());
    }
    draw(bmp);
    fs::create_directories(cache_dir_);
    bmp.save_to_file((
        fs::path{ cache_dir_ } /
        ("particles-" +
        std::to_string(image_frames_.rbegin()->first.count()) +
        ".png")).string());
}

void FlowingParticles::draw(StbImage3& bmp) {
    assert(all(bmp.fixed_shape<2>() == shape_));
    assert(particles_.size() > 0);
    for (const auto& s : particles_.rbegin()->second.tracked_points) {
        FixedArray<size_t, 2> index{ a2i(s.second->sequence.rbegin()->second->position) };
        //assert(all(index < bmp.shape()));
        auto sq_res_it = last_sq_residual_.find(s.first);
        float marker_size = sq_res_it == last_sq_residual_.end()
            ? 1.f
            : std::sqrt(sq_res_it->second);
        bmp.draw_empty_rect(
            index,
            (bad_points_.find(s.first) == bad_points_.end()
                ? std::max(size_t{ 2 }, (size_t)std::round(marker_size))
                : 4),
            (bad_points_.find(s.first) == bad_points_.end()
                ? Rgb24::green()
                : Rgb24::black()));
        if (optical_flow_frames_.size() >= 1) {
            bmp.draw_streamline(
                index,
                optical_flow_frames_.rbegin()->second.grayscale,
                1,
                streamline_search_length(),
                Rgb24::blue());
        }
    }
}

bool FlowingParticles::requires_optical_flow() const {
    return
        (cfg_.tracking_mode != TrackingMode::PATCH_NEW_POSITION_IN_BOX &&
         !(cfg_.tracking_mode & TrackingMode::SIFT)) ||
        cfg_.draw_optical_flow;
}
