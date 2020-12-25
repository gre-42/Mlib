#include "Flowing_Particles.hpp"
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/OpenCV.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Stats/Min_Max.hpp>

#if 0
#endif

using namespace Mlib;
using namespace Mlib::Sfm;

FlowingParticles::FlowingParticles(
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
    const std::map<std::chrono::milliseconds, ImageFrame>& optical_flow_frames,
    const std::string& cache_dir,
    const FlowingParticlesConfig& cfg)
: image_frames_{image_frames},
  optical_flow_frames_{optical_flow_frames},
#if 0
  orb_{cv::ORB::create(cfg_.target_nparticles)},
#endif
  shape_{SIZE_MAX},
  cache_dir_{cache_dir},
  cfg_{cfg}
{}

void FlowingParticles::generate_new_particles(FeaturePointFrame& new_frame) {
    assert(image_frames_.size() > 0);
    shape_ = image_frames_.rbegin()->second.grayscale.shape();
    assert(new_frame.size() <= cfg_.target_nparticles);
    const size_t n_new_particles = cfg_.target_nparticles - new_frame.size();

    if (false) {
        for (size_t r = 0; r < shape_(id0); r+=shape_(id0)/10) {
            for (size_t c = 0; c < shape_(id1); c+=shape_(id1)/10) {
                try_generate_feature_point_sequence(
                    new_frame,
                    i2a(ArrayShape{r, c}));
            }
        }
    }

    Array<bool> existing_points_mask = ones<bool>(image_frames_.rbegin()->second.grayscale.shape());
    for (const auto& kv : new_frame) {
        draw_fill_rect(
            existing_points_mask,
            a2i(kv.second->sequence.rbegin()->second->position),
            5,      //size
            false); //value
    }

    Array<float> points;
    if (false) {
        Array<float> h_trace;
        Array<float> h_det;
        float r_thr = 10;
        hessian_determinant_trace(box_filter(image_frames_.rbegin()->second.grayscale, ArrayShape{10, 10}, NAN), &h_det, &h_trace);
        Array<float> R = squared(h_trace) / (h_det + float(1e-6));
        points = find_nfeatures(-harris_response(
            image_frames_.rbegin()->second.grayscale),
            existing_points_mask && (R < squared(r_thr + 1) / r_thr),
            n_new_particles);
        //Bgr565Bitmap::from_float_grayscale(
        //    substitute_nans(
        //        normalized_and_clipped(
        //            R, 0.f, squared(r_thr + 1) / r_thr),
        //        0.f))
        //    .save_to_file("edges.bmp");
    } else if (true) {
        Array<bool> feature_mask;
        Array<float> hr = harris_response(image_frames_.rbegin()->second.grayscale, &feature_mask);
        points = find_nfeatures(
            -hr,

            // normalize_brightness(
            //     -hr,
            //     ArrayShape{100, 100},
            //     0.f),

            // -hr /
            // (box_filter(image_frames_.rbegin()->second.grayscale, ArrayShape{3, 3}, NAN) + 10.f),

            // abs(laplace_filter(image_frames_.rbegin()->second.grayscale, NAN)),

            existing_points_mask && feature_mask,
            n_new_particles,
            cfg_.distance_sigma);
    } else if (false) {
        Array<bool> mask = multi_scale_harris(image_frames_.rbegin()->second.grayscale, 5);
        points = find_nfeatures(
            -harris_response(image_frames_.rbegin()->second.grayscale),
            existing_points_mask && mask,
            n_new_particles);
        //std::cerr << points.shape() << std::endl;
        //std::cerr << count_nonzero(mask) << std::endl;
    }
    std::cerr << "Trying to generate " << points.shape(0) << " new particles" << std::endl;
    for (const Array<float>& p : points) {
        try_generate_feature_point_sequence(new_frame, p);
    }
}

size_t FlowingParticles::streamline_search_length() {
    assert(image_frames_.size() > 0);
    return (5 * max(Array<size_t>::from_shape(
        image_frames_.rbegin()->second.grayscale.shape()))) / 113;
}

std::shared_ptr<FeaturePoint> FlowingParticles::generate_feature_point(const Array<float>& pos) const
{
    return std::make_shared<FeaturePoint>(
        pos,
        TraceablePatch(
            image_frames_.rbegin()->second.rgb,
            ArrayShape{a2i(pos)},
            cfg_.patch_size));
}

bool FlowingParticles::try_generate_feature_point_sequence(
    FeaturePointFrame& frame,
    const Array<float>& pos)
{
    auto seq = std::make_shared<FeaturePointSequence>();
    auto p = generate_feature_point(pos);
    if (p->traceable_patch.good_) {
        seq->sequence[image_frames_.rbegin()->first] = p;
        frame[sequence_index++] = seq;
        return true;
    }
    return false;
}

bool FlowingParticles::try_insert_and_append_feature_point(
    FeaturePointFrame& frame,
    const std::pair<size_t, std::shared_ptr<FeaturePointSequence>>& seq,
    const Array<float>& pos)
{
    auto p = generate_feature_point(pos);
    if (p->traceable_patch.good_) {
        seq.second->sequence[image_frames_.rbegin()->first] = p;
        frame.insert(seq);
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
        assert(all(flow_field.shape().erased_first() == shape_));
        assert(flow_field.shape(0) == 2);
    }
    size_t nsuccess = 0;
    for (const auto& s : particles_.rbegin()->second) {
        //if (bad_points_.find(s.first) != bad_points_.end()) {
        //    std::cerr << "Discontinuing bad point [" << s.first << "]" << std::endl;
        //    continue;
        //}
        const FeaturePoint& p = *s.second->sequence.rbegin()->second;
        ArrayShape index{a2i(p.position)};
        if (all(index < shape_)) {
            Array<float> new_pos;
            if (false) {
                if (!flow_mask(index)) {
                    continue;
                }
                Array<float> v{
                    flow_field[0](index),
                    flow_field[1](index)};
                new_pos = find_feature_in_neighborhood(p.position + v, -hr, ArrayShape{5, 5});
            } else if (false) {
                if (!flow_mask(index)) {
                    continue;
                }
                float best_harris = std::numeric_limits<float>::infinity();
                visit_streamline(shape_, index, flow_field, streamline_search_length(), [&](const ArrayShape& id) {
                    if (hr(id) < best_harris) {
                        best_harris = hr(id);
                        new_pos = i2a(id);
                    }
                });
            }
            if (cfg_.tracking_mode == TrackingMode::PATCH_NEW_POSITION_IN_BOX) {
                ArrayShape new_index = s.second->sequence.begin()->second->traceable_patch.new_position_in_box(
                    image_frames_.rbegin()->second.rgb,
                    a2i(p.position),
                    cfg_.search_window,
                    cfg_.worst_patch_error);
                new_pos = i2a(new_index);
                if (any(new_pos > float(1e6))) {
                    // std::cerr << "dropping y [" << s.first << "]" << std::endl;
                    continue;
                }
            }
            if (try_insert_and_append_feature_point(new_frame, s, new_pos)) {
                ++nsuccess;
            }
        }
    }
    std::cerr << "Traced " << nsuccess << " / " << particles_.rbegin()->second.size() << " particles" << std::endl;
}

void FlowingParticles::advance_flowing_particles() {
    assert(image_frames_.size() >= 1);
    FeaturePointFrame new_frame;
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
    particles_.insert(std::make_pair(image_frames_.rbegin()->first, new_frame));
    Bgr565Bitmap bmp = Bgr565Bitmap::from_float_grayscale(image_frames_.rbegin()->second.grayscale);
    if (optical_flow_frames_.size() >= 1) {
        bmp.draw_mask(optical_flow_frames_.rbegin()->second.mask, Bgr565::red());
    }
    draw(bmp);
    bmp.save_to_file(
        cache_dir_ +
        "/particles-" +
        std::to_string(image_frames_.rbegin()->first.count()) +
        ".bmp");
}

#if 0
void FlowingParticles::advance_flowing_particles_cv() {
    assert(image_frames_.size() >= 1);
    FeaturePointFrame new_frame;
    shape_ = image_frames_.rbegin()->second.grayscale.shape();

    std::vector<cv::KeyPoint> keypoints1t;
    cv::Mat_<uint8_t> descriptors1t;
    orb_->detectAndCompute(
        array_to_cv_mat((clipped(image_frames_.rbegin()->second.grayscale, 0.f, 1.f) * 255.f).casted<uint8_t>()),
        cv::noArray(),
        keypoints1t,
        descriptors1t);
    Array<uint8_t> des1t = cv_mat_to_array(descriptors1t);
    std::set<size_t> unmatched_ids1t;
    for (size_t i = 0; i < des1t.shape(0); ++i) {
        unmatched_ids1t.insert(i);
    }
    if (particles_.size() > 0) {
        Array<uint8_t> des0q{ArrayShape{particles_.rbegin()->second.size(), des1t.shape(1)}};
        std::vector<size_t> ids0q;
        for (const auto& p0 : particles_.rbegin()->second) {
            des0q[ids0q.size()] = p0.second->sequence.begin()->second->descriptor;
            ids0q.push_back(p0.first);
        }
        std::vector<cv::DMatch> matches;
        bfm_.match(array_to_cv_mat(des0q), descriptors1t, matches);
        // std::sort(matches.begin(), matches.end());
        std::cerr << "Matched " << matches.size() << " existing particles" << std::endl;
        for (const auto& m : matches) {
            unmatched_ids1t.erase(m.trainIdx);
            try_insert_and_append_feature_point(
                new_frame,
                *particles_.rbegin()->second.find(ids0q.at(m.queryIdx)),
                Array<float>{keypoints1t.at(m.trainIdx).pt.x, keypoints1t.at(m.trainIdx).pt.y},
                cv_mat_to_array(descriptors1t.row(m.trainIdx)));
        }
    }
    std::cerr << "Trying to generate " << unmatched_ids1t.size() << " new particles" << std::endl;
    for (size_t i1t : unmatched_ids1t) {
        try_generate_feature_point_sequence(
            new_frame,
            Array<float>{keypoints1t.at(i1t).pt.x, keypoints1t.at(i1t).pt.y},
            des1t[i1t]);
    }

    particles_.insert(std::make_pair(image_frames_.rbegin()->first, new_frame));
    Bgr565Bitmap bmp = Bgr565Bitmap::from_float_grayscale(image_frames_.rbegin()->second.grayscale);
    if (optical_flow_frames_.size() >= 1) {
        bmp.draw_mask(optical_flow_frames_.rbegin()->second.mask, Bgr565::red());
    }
    draw(bmp);
    bmp.save_to_file(
        cache_dir_ +
        "/particles-" +
        std::to_string(image_frames_.rbegin()->first.count()) +
        ".bmp");
}
#endif

void FlowingParticles::draw(Bgr565Bitmap& bmp) {
    assert(all(bmp.shape() == shape_));
    assert(particles_.size() > 0);
    for (const auto& s : particles_.rbegin()->second) {
        ArrayShape index{a2i(s.second->sequence.rbegin()->second->position)};
        //assert(all(index < bmp.shape()));
        bmp.draw_fill_rect(
            index,
            (bad_points_.find(s.first) == bad_points_.end()
                ? 2
                : 4),
            (bad_points_.find(s.first) == bad_points_.end()
                ? Bgr565::green()
                : Bgr565::black()));
        if (optical_flow_frames_.size() >= 1) {
            bmp.draw_streamline(
                index,
                optical_flow_frames_.rbegin()->second.grayscale,
                1,
                streamline_search_length(),
                Bgr565::blue());
        }
    }
}

bool FlowingParticles::requires_optical_flow() const {
    return
        cfg_.tracking_mode != TrackingMode::PATCH_NEW_POSITION_IN_BOX ||
        cfg_.draw_optical_flow;
}
