#include "Load_Bvh.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <fstream>

using namespace Mlib;

FixedArray<float, 4, 4> Mlib::get_parameter_transformation(const std::string& name) {
    if (name == "xz-y") {
        return FixedArray<float, 4, 4>{
            1.f,  0.f, 0.f, 0.f,
            0.f,  0.f, 1.f, 0.f,
            0.f, -1.f, 0.f, 0.f,
            0.f,  0.f, 0.f, 1.f};
    } else {
        THROW_OR_ABORT("Unknown parameter transformation: " + name);
    }
};

BvhLoader::BvhLoader(
    const std::string& filename,
    const BvhConfig& cfg)
: cfg_{cfg},
  frame_time_{NAN}
{
    if (!all(cfg.rotation_order < size_t(3))) {
        THROW_OR_ABORT("Rotation order out of bounds");
    }
    std::ifstream f{filename};
    if (f.fail()) {
        THROW_OR_ABORT("Could not open " + filename);
    }
    std::vector<std::map<std::string, FixedArray<float, 2, 3>>> raw_frames;
    std::string line;
    std::string joint_name;
    std::list<std::string> joint_stack;
    bool in_data_section = false;
    size_t nframes = SIZE_MAX;
    while (std::getline(f, line)) {
        if (!in_data_section) {
            static const DECLARE_REGEX(name_re, "^\\s*(ROOT|JOINT)\\s+(.+?)\\s*$");
            static const DECLARE_REGEX(closing_re, "^\\s*}\\s*$");
            static const DECLARE_REGEX(ends_re, "^\\s*End Site\\s*$");
            static const DECLARE_REGEX(offs_re, "^\\s*OFFSET\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s*$");
            static const DECLARE_REGEX(chan_re, "^\\s*CHANNELS\\s+(\\d+)\\s+(.+)\\s*$");
            static const DECLARE_REGEX(motion_re, "^\\s*MOTION\\s*$");
            Mlib::re::smatch match;
            if (Mlib::re::regex_match(line, match, name_re)) {
                joint_name = match[2].str();
                if ((match[1].str() == "ROOT") != joint_stack.empty()) {
                    THROW_OR_ABORT("Inconsistent ROOT hierarchy");
                }
                if (!joint_stack.empty() && !parents_.insert({joint_name, joint_stack.back()}).second)
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
                    if (!offsets_.insert({joint_name, FixedArray<float, 3>{
                        safe_stof(match[1].str()),
                        safe_stof(match[2].str()),
                        safe_stof(match[3].str())}}).second)
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
                if (!std::getline(f, line)) {
                    THROW_OR_ABORT("Could not get line");
                }
                if (!Mlib::re::regex_match(line, match, frames_re)) {
                    THROW_OR_ABORT("Could not match frames: \"" + line + '"');
                }
                nframes = safe_stoz(match[1].str());
                raw_frames.reserve(nframes);
                if (!std::getline(f, line)) {
                    THROW_OR_ABORT("Could not get line");
                }
                if (!Mlib::re::regex_match(line, match, frame_time_re)) {
                    THROW_OR_ABORT("Could not match frame time: \"" + line + '"');
                }
                frame_time_ = safe_stof(match[1].str()) * s;
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
            for (const auto& o : offsets_) {
                frame[o.first][0] = o.second;
            }
            size_t i = 0;
            for (const auto& c : columns_) {
                frame[c.joint_name](c.pose_index0, c.pose_index1) = d[i++];
                //  + offsets_.at(c.joint_name)(c.pose_index0, c.pose_index1);
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
            center += f.at(columns_.begin()->joint_name)[0];
        }
        center /= (float)raw_frames.size();
        for (auto& f : raw_frames) {
            f.at(columns_.begin()->joint_name)[0] -= center;
        }
    }
    if (std::isnan(frame_time_)) {
        THROW_OR_ABORT("Frame time not set");
    }
    transformed_frames_.resize(raw_frames.size() + (cfg.periodic && !raw_frames.empty()));
    for (size_t i = 0; i < raw_frames.size(); ++i) {
        for (const auto& p : raw_frames[i]) {
            const FixedArray<float, 3>& position = p.second[0];
            const FixedArray<float, 3>& rotation = p.second[1];
            FixedArray<float, 4, 4> m = assemble_homogeneous_4x4(
                tait_bryan_angles_2_matrix(
                    rotation * degrees,
                    cfg_.rotation_order),
                position * cfg_.scale);
            const FixedArray<float, 4, 4>& n = cfg_.parameter_transformation;
            if (!transformed_frames_[i].insert({
                p.first,
                OffsetAndQuaternion<float, float>{dot2d(n, dot2d(m, n.T()))}}).second)
            {
                THROW_OR_ABORT("Could not insert transformed frame");
            }
        }
    }
    smoothen();
    if (cfg.periodic && !raw_frames.empty()) {
        transformed_frames_[transformed_frames_.size() - 1] = transformed_frames_[0];
    }
}

const std::map<std::string, OffsetAndQuaternion<float, float>>& BvhLoader::get_frame(size_t id) const {
    if (id >= transformed_frames_.size()) {
        THROW_OR_ABORT("Frame index too large");
    }
    return transformed_frames_[id];
}

std::map<std::string, OffsetAndQuaternion<float, float>> BvhLoader::get_relative_interpolated_frame(float time) const {
    if (transformed_frames_.empty()) {
        THROW_OR_ABORT("No frames to interpolate from");
    }
    float i = time / frame_time_;
    i = std::clamp(i, float{0}, float(transformed_frames_.size() - 1));
    size_t i0 = size_t(i);
    size_t i1 = i0 + 1;
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
    std::map<std::string, OffsetAndQuaternion<float, float>> result;
    for (const auto& j0 : f0) {
        const auto& m0 = j0.second;
        const auto& m1 = f1.at(j0.first);
        if (!result.insert({j0.first, m0.slerp(m1, a0)}).second) {
            THROW_OR_ABORT("Could not insert interpolated frame");
        }
    }
    return result;
}

void BvhLoader::compute_absolute_transformation(
    const std::string& name,
    const std::map<std::string, OffsetAndQuaternion<float, float>>& relative_transformations,
    std::map<std::string, OffsetAndQuaternion<float, float>>& absolute_transformations,
    size_t ncalls) const
{
    if (absolute_transformations.contains(name)) {
        return;
    }
    auto it = parents_.find(name);
    if (it == parents_.end()) {
        absolute_transformations[name] = relative_transformations.at(name);
    } else {
        if (ncalls > 100) {
            for (const auto& [n, p] : parents_) {
                std::cerr << n << " -> " << p << std::endl;
            }
            THROW_OR_ABORT("Recursion depth exceeded, probably loop in parents mapping");
        }
        compute_absolute_transformation(
            it->second,
            relative_transformations,
            absolute_transformations,
            ncalls + 1);
        absolute_transformations[name] = absolute_transformations.at(it->second) * relative_transformations.at(name);
    }
}

std::map<std::string, OffsetAndQuaternion<float, float>> BvhLoader::get_absolute_interpolated_frame(float time) const {
    auto rel = get_relative_interpolated_frame(time);
    std::map<std::string, OffsetAndQuaternion<float, float>> result;
    for (const auto& [name, _] : rel) {
        compute_absolute_transformation(name, rel, result, 0);
    }
    return result;
}

int mod(int x, int N) {
    return (x % N + N) % N;
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
    std::vector<std::map<std::string, OffsetAndQuaternion<float, float>>> smoothed_transformed_frames((transformed_frames_.size() - 1));
    const auto& get_cyclic_frame = [this](int i){
        return transformed_frames_.at((size_t)mod(i, (int)(transformed_frames_.size() - 1)));
    };
    for (int t = 0; t < (int)(transformed_frames_.size() - 1); ++t) {
        std::map<std::string, OffsetAndQuaternion<float, float>> fn = get_cyclic_frame(t - (int)cfg_.smooth_radius);
        for (int i = t - (int)cfg_.smooth_radius + 1; i < t; ++i) {
            for (const auto& f : get_cyclic_frame(i)) {
                fn.at(f.first) = fn.at(f.first).slerp(f.second, cfg_.smooth_alpha);
            }
        }
        std::map<std::string, OffsetAndQuaternion<float, float>> fp = get_cyclic_frame(t + (int)cfg_.smooth_radius);
        for (int i = t + (int)cfg_.smooth_radius - 1; i > t; --i) {
            for (const auto& f : get_cyclic_frame(i)) {
                fp.at(f.first) = fp.at(f.first).slerp(f.second, cfg_.smooth_alpha);
            }
        }
        for (const auto& en : fn) {
            smoothed_transformed_frames.at((size_t)t)[en.first] = en.second.slerp(fp.at(en.first), 0.5);
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
