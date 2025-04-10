#include "Load_Bvh.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <algorithm>
#include <istream>

using namespace Mlib;

FixedArray<float, 4, 4> Mlib::get_parameter_transformation(const std::string& name) {
    if (name == "xz-y") {
        return x_z_my;
    } else {
        THROW_OR_ABORT("Unknown parameter transformation: " + name);
    }
}

BvhLoader::BvhLoader(
    const std::string& filename,
    const BvhConfig& cfg)
    : cfg_{ cfg }
    , frame_time_{ NAN }
{
    if (!all(cfg.rotation_order < size_t(3))) {
        THROW_OR_ABORT("Rotation order out of bounds");
    }
    auto file = create_ifstream(filename);
    if (file->fail()) {
        THROW_OR_ABORT("Could not open \"" + filename + '"');
    }
    std::vector<Map<std::string, FixedArray<float, 2, 3>>> raw_frames;
    std::string line;
    std::string joint_name;
    std::list<std::string> joint_stack;
    bool in_data_section = false;
    size_t nframes = SIZE_MAX;
    while (std::getline(*file, line)) {
        if (!in_data_section) {
            static const DECLARE_REGEX(name_re, "^\\s*(ROOT|JOINT)\\s+(.+?)\\s*$");
            static const DECLARE_REGEX(closing_re, "^\\s*\\}\\s*$");
            static const DECLARE_REGEX(ends_re, "^\\s*End Site\\s*$");
            static const DECLARE_REGEX(offs_re, "^\\s*OFFSET\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s*$");
            static const DECLARE_REGEX(chan_re, "^\\s*CHANNELS\\s+(\\d+)\\s+(.+)\\s*$");
            static const DECLARE_REGEX(motion_re, "^\\s*MOTION\\s*$");
            Mlib::re::cmatch match;
            if (Mlib::re::regex_match(line, match, name_re)) {
                joint_name = match[2].str();
                if ((match[1].str() == "ROOT") != joint_stack.empty()) {
                    THROW_OR_ABORT("Inconsistent ROOT hierarchy");
                }
                if (!joint_stack.empty() && !parents_.try_emplace(joint_name, joint_stack.back()).second)
                {
                    THROW_OR_ABORT("Parent of \"" + joint_name + "\" already set");
                }
                joint_stack.push_back(joint_name);
            } else if (Mlib::re::regex_match(line, match, closing_re)) {
                if (joint_stack.empty()) {
                    THROW_OR_ABORT("Joint stack is empty despite \"}\"");
                }
                joint_stack.pop_back();
            } else if (Mlib::re::regex_match(line, match, ends_re)) {
                joint_name = "<end site>";
                joint_stack.push_back(joint_name);
            } else if (Mlib::re::regex_match(line, match, offs_re)) {
                if (joint_name != "<end site>") {
                    if (!offsets_.try_emplace(
                        joint_name,
                        safe_stof(match[1].str()),
                        safe_stof(match[2].str()),
                        safe_stof(match[3].str())).second)
                    {
                        THROW_OR_ABORT("Could not insert offset for joint " + joint_name);
                    }
                }
            } else if (Mlib::re::regex_match(line, match, chan_re)) {
                size_t nchannels = safe_stoz(match[1].str());
                auto channels = string_to_vector(match[2].str());
                if (channels.size() != nchannels) {
                    THROW_OR_ABORT("Channel number mismatch");
                }
                for (const auto& c : channels) {
                    size_t cid0 = SIZE_MAX;
                    size_t cid1 = SIZE_MAX;
                    if (c == "Xposition") {
                        cid0 = 0;
                        cid1 = 0;
                    } else if (c == "Yposition") {
                        cid0 = 0;
                        cid1 = 1;
                    } else if (c == "Zposition") {
                        cid0 = 0;
                        cid1 = 2;
                    } else if (c == "Xrotation") {
                        cid0 = 1;
                        cid1 = 0;
                    } else if (c == "Yrotation") {
                        cid0 = 1;
                        cid1 = 1;
                    } else if (c == "Zrotation") {
                        cid0 = 1;
                        cid1 = 2;
                    } else {
                        THROW_OR_ABORT("Unknown channel type");
                    }
                    columns_.push_back(ColumnDescription{.joint_name = joint_name, .pose_index0 = cid0, .pose_index1 = cid1});
                }
            } else if (Mlib::re::regex_match(line, match, motion_re)) {
                static const DECLARE_REGEX(frames_re, "^\\s*Frames:\\s*(\\d+)\\s*");
                static const DECLARE_REGEX(frame_time_re, "^\\s*Frame Time:\\s*(\\S+)\\s*");
                if (!std::getline(*file, line)) {
                    THROW_OR_ABORT("Could not get line");
                }
                if (!Mlib::re::regex_match(line, match, frames_re)) {
                    THROW_OR_ABORT("Could not match frames: \"" + line + '"');
                }
                nframes = safe_stoz(match[1].str());
                raw_frames.reserve(nframes);
                if (!std::getline(*file, line)) {
                    THROW_OR_ABORT("Could not get line");
                }
                if (!Mlib::re::regex_match(line, match, frame_time_re)) {
                    THROW_OR_ABORT("Could not match frame time: \"" + line + '"');
                }
                frame_time_ = safe_stof(match[1].str()) * seconds;
                in_data_section = true;
            }
        } else {
            std::vector<float> d = string_to_vector(line, safe_stof);
            if (d.size() != columns_.size()) {
                THROW_OR_ABORT(
                    "Invalid data length. Expected " + std::to_string(columns_.size()) +
                    ", received " + std::to_string(d.size()) +
                    ": " + line);
            }
            if (nframes == SIZE_MAX) {
                THROW_OR_ABORT("In data section without nframes");
            }
            if (raw_frames.size() == nframes) {
                THROW_OR_ABORT("Too many frames in BVH file");
            }
            auto& frame = raw_frames.emplace_back();
            for (const auto& [joint_name, offset] : offsets_) {
                frame.add(joint_name, fixed_nans<float, 2, 3>());
                frame.at(joint_name)[0] = offset;
            }
            for (const auto& [i, c] : enumerate(columns_)) {
                frame.get(c.joint_name)(c.pose_index0, c.pose_index1) = d[i];
                //  + offsets_.get(c.joint_name)(c.pose_index0, c.pose_index1);
            }
            for (const auto& [n, v] : frame) {
                if (any(isnan(v))) {
                    THROW_OR_ABORT("Detected NAN value in joint \"" + n + '"');
                }
            }
        }
    }
    if (!joint_stack.empty()) {
        THROW_OR_ABORT("Joint stack not empty");
    }
    if (nframes == SIZE_MAX) {
        THROW_OR_ABORT("nframes undefined");
    }
    if (raw_frames.size() != nframes) {
        THROW_OR_ABORT("Too few frames in BVH file");
    }
    if (cfg_.demean) {
        if (columns_.empty()) {
            THROW_OR_ABORT("Columns list is empty");
        }
        FixedArray<float, 3> center(0.f);
        for (const auto& f : raw_frames) {
            center += f.get(columns_.begin()->joint_name)[0];
        }
        center /= (float)raw_frames.size();
        for (auto& f : raw_frames) {
            f.get(columns_.begin()->joint_name)[0] -= center;
        }
    }
    if (std::isnan(frame_time_)) {
        THROW_OR_ABORT("Frame time not set");
    }
    transformed_frames_.resize(raw_frames.size() + (cfg.periodic && !raw_frames.empty()));
    for (size_t i = 0; i < raw_frames.size(); ++i) {
        for (const auto& [joint_name, transformation] : raw_frames[i]) {
            const auto& position = transformation[0];
            const auto& rotation = transformation[1];
            auto m = assemble_homogeneous_4x4(
                tait_bryan_angles_2_matrix(
                    rotation * degrees,
                    cfg_.rotation_order),
                position * cfg_.scale);
            const auto& n = cfg_.parameter_transformation;
            if (!transformed_frames_.at(i).try_emplace(
                joint_name,
                dot2d(n, dot2d(m, n.T()))).second)
            {
                THROW_OR_ABORT("Could not insert transformed frame");
            }
        }
    }
    smoothen();
    if (cfg.periodic && !raw_frames.empty()) {
        transformed_frames_[transformed_frames_.size() - 1] = transformed_frames_[0];
    }
    if (!file->eof() && file->fail()) {
        THROW_OR_ABORT("Error reading from file: \"" + filename + '"');
    }
}

const Map<std::string, OffsetAndQuaternion<float, float>>& BvhLoader::get_frame(size_t id) const {
    if (id >= transformed_frames_.size()) {
        THROW_OR_ABORT("Frame index too large");
    }
    return transformed_frames_[id];
}

Map<std::string, OffsetAndQuaternion<float, float>> BvhLoader::get_relative_interpolated_frame(float time) const {
    if (transformed_frames_.empty()) {
        THROW_OR_ABORT("No frames to interpolate from");
    }
    float i = time / frame_time_;
    i = std::clamp(i, float{0}, float(transformed_frames_.size() - 1));
    auto i0 = size_t(i);
    auto i1 = i0 + 1;
    if (i1 > transformed_frames_.size()) {
        THROW_OR_ABORT("Frame interpolation internal error");
    }
    if (i1 == transformed_frames_.size()) {
        --i1;
        --i0;
    }
    float a0 = i - float(i0);
    const auto& f0 = get_frame(i0);
    const auto& f1 = get_frame(i1);
    Map<std::string, OffsetAndQuaternion<float, float>> result;
    for (const auto& j0 : f0) {
        const auto& m0 = j0.second;
        const auto& m1 = f1.get(j0.first);
        if (!result.insert({j0.first, m0.slerp(m1, a0)}).second) {
            THROW_OR_ABORT("Could not insert interpolated frame");
        }
    }
    return result;
}

void BvhLoader::compute_absolute_transformation(
    const std::string& name,
    const Map<std::string, OffsetAndQuaternion<float, float>>& relative_transformations,
    Map<std::string, OffsetAndQuaternion<float, float>>& absolute_transformations,
    size_t ncalls) const
{
    if (absolute_transformations.contains(name)) {
        return;
    }
    auto it = parents_.find(name);
    if (it == parents_.end()) {
        if (!absolute_transformations.try_emplace(name, relative_transformations.get(name)).second) {
            verbose_abort("compute_absolute_transformation internal error");
        }
    } else {
        if (ncalls > 100) {
            for (const auto& [n, p] : parents_) {
                lerr() << n << " -> " << p;
            }
            THROW_OR_ABORT("Recursion depth exceeded, probably loop in parents mapping");
        }
        compute_absolute_transformation(
            it->second,
            relative_transformations,
            absolute_transformations,
            ncalls + 1);
        absolute_transformations.add(name, absolute_transformations.get(it->second) * relative_transformations.get(name));
    }
}

Map<std::string, OffsetAndQuaternion<float, float>> BvhLoader::get_absolute_interpolated_frame(float time) const {
    auto rel = get_relative_interpolated_frame(time);
    Map<std::string, OffsetAndQuaternion<float, float>> result;
    for (const auto& [name, _] : rel) {
        compute_absolute_transformation(name, rel, result, 0);
    }
    return result;
}

static int mod(int x, int n) {
    return (x % n + n) % n;
}

void BvhLoader::smoothen() {
    if (transformed_frames_.empty()) {
        return;
    }
    if (cfg_.smooth_radius == 0) {
        return;
    }
    if (!cfg_.periodic) {
        THROW_OR_ABORT("Aperiodic smoothing not implemented");
    }
    std::vector<Map<std::string, OffsetAndQuaternion<float, float>>>
        smoothed_transformed_frames(transformed_frames_.size());
    // `transformed_frames_.size() - 1` because of the line
    // `transformed_frames_.resize(raw_frames.size() + (cfg.periodic && !raw_frames.empty()));`
    // above. Also, there is a check for periodicity above.
    auto N = integral_cast<int>(transformed_frames_.size() - 1);
    auto r = integral_cast<int>(cfg_.smooth_radius);
    const auto& get_cyclic_frame = [&](int i) {
        return transformed_frames_.at(integral_cast<size_t>(mod(i, N)));
    };
    for (int t = 0; t < N; ++t) {
        auto fn = get_cyclic_frame(t - r);
        for (int i = t - r + 1; i < t; ++i) {
            for (const auto& [joint_name, transformation] : get_cyclic_frame(i)) {
                fn.get(joint_name) = fn.get(joint_name).slerp(transformation, cfg_.smooth_alpha);
            }
        }
        auto fp = get_cyclic_frame(t + r);
        for (int i = t + r - 1; i > t; --i) {
            for (const auto& [joint_name, transformation] : get_cyclic_frame(i)) {
                fp.get(joint_name) = fp.get(joint_name).slerp(transformation, cfg_.smooth_alpha);
            }
        }
        for (const auto& [joint_name, transformation] : fn) {
            smoothed_transformed_frames.at(integral_cast<size_t>(t)).add(joint_name, transformation.slerp(fp.get(joint_name), 0.5));
        }
    }
    transformed_frames_ = std::move(smoothed_transformed_frames);
}

float BvhLoader::duration() const {
    if (transformed_frames_.size() < 2) {
        THROW_OR_ABORT("Computation of animation duration requires at least two frames");
    }
    return frame_time_ * float(transformed_frames_.size() - 1);
}
