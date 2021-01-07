#include "Load_Bvh.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/String.hpp>
#include <fstream>
#include <regex>

using namespace Mlib;

FixedArray<float, 4, 4> Mlib::get_parameter_transformation(const std::string& name) {
    if (name == "xz-y") {
        return FixedArray<float, 4, 4>{
            1, 0, 0, 0,
            0, 0, 1, 0,
            0, -1, 0, 0,
            0, 0, 0, 1};
    } else {
        throw std::runtime_error("Unknown parameter transformation: " + name);
    }
};

BvhLoader::BvhLoader(
    const std::string& filename,
    const BvhConfig& cfg)
: cfg_{cfg},
  frame_time_{NAN}
{
    if (!all(cfg.rotation_order < size_t(3))) {
        throw std::runtime_error("Rotation order out of bounds");
    }
    std::ifstream f{filename};
    if (f.fail()) {
        throw std::runtime_error("Could not open " + filename);
    }
    std::vector<std::map<std::string, FixedArray<float, 2, 3>>> raw_frames;
    std::string line;
    std::string joint_name;
    bool in_data_section = false;
    size_t nframes = SIZE_MAX;
    while (std::getline(f, line)) {
        if (!in_data_section) {
            static const std::regex name_re{"^[\\s\\n\\r]*(?:ROOT|JOINT)\\s+(\\w+)[\\s\\n\\r]*$"};
            static const std::regex ends_re{"^[\\s\\n\\r]*End Site[\\s\\n\\r]*$"};
            static const std::regex offs_re{"^[\\s\\n\\r]*OFFSET\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)[\\n\\r]*$"};
            static const std::regex chan_re{"^[\\s\\n\\r]*CHANNELS\\s+(\\d+)\\s+(.+)[\\n\\r]*$"};
            static const std::regex motion_re{"^[\\s\\n\\r]*MOTION[\\s\\n\\r]*$"};
            std::smatch match;
            if (std::regex_match(line, match, name_re)) {
                joint_name = match[1].str();
            } else if (std::regex_match(line, match, ends_re)) {
                joint_name = "<end site>";
            } else if (std::regex_match(line, match, offs_re)) {
                if (joint_name != "<end site>") {
                    if (!offsets_.insert({joint_name, FixedArray<float, 3>{
                        safe_stof(match[1].str()),
                        safe_stof(match[2].str()),
                        safe_stof(match[3].str())}}).second)
                    {
                        throw std::runtime_error("Could not insert offset for joint " + joint_name);
                    }
                }
            } else if (std::regex_match(line, match, chan_re)) {
                size_t nchannels = safe_stoz(match[1].str());
                auto channels = string_to_vector(match[2].str());
                if (channels.size() != nchannels) {
                    throw std::runtime_error("Channel number mismatch");
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
                        throw std::runtime_error("Unknown channel type");
                    }
                    columns_.push_back(ColumnDescription{.joint_name = joint_name, .pose_index0 = cid0, .pose_index1 = cid1});
                }
            } else if (std::regex_match(line, match, motion_re)) {
                static const std::regex frames_re{"^\\s*Frames:\\s*(\\d+)\\s*"};
                static const std::regex frame_time_re{"^\\s*Frame Time:\\s*(\\S+)\\s*"};
                if (!std::getline(f, line)) {
                    throw std::runtime_error("Could not get line");
                }
                if (!std::regex_match(line, match, frames_re)) {
                    throw std::runtime_error("Could not match frames: \"" + line + '"');
                }
                nframes = safe_stoz(match[1].str());
                raw_frames.reserve(nframes);
                if (!std::getline(f, line)) {
                    throw std::runtime_error("Could not get line");
                }
                if (!std::regex_match(line, match, frame_time_re)) {
                    throw std::runtime_error("Could not match frame time: \"" + line + '"');
                }
                frame_time_ = safe_stof(match[1].str());
                in_data_section = true;
            }
        } else {
            std::vector<float> d = string_to_vector(line, safe_stof);
            if (d.size() != columns_.size()) {
                throw std::runtime_error(
                    "Invalid data length. Expected " + std::to_string(columns_.size()) +
                    ", received " + std::to_string(d.size()) +
                    ": " + line);
            }
            if (nframes == SIZE_MAX) {
                throw std::runtime_error("In data section without nframes");
            }
            if (raw_frames.size() == nframes) {
                throw std::runtime_error("Too many frames in BVH file");
            }
            raw_frames.emplace_back();
            for (const auto& o : offsets_) {
                raw_frames.back()[o.first][0] = o.second;
            }
            size_t i = 0;
            for (const auto& c : columns_) {
                raw_frames.back()[c.joint_name](c.pose_index0, c.pose_index1) = d[i++];
                //  + offsets_.at(c.joint_name)(c.pose_index0, c.pose_index1);
            }
        }
    }
    if (nframes == SIZE_MAX) {
        throw std::runtime_error("nframes undefined");
    }
    if (raw_frames.size() != nframes) {
        throw std::runtime_error("Too few frames in BVH file");
    }
    if (cfg_.demean) {
        if (columns_.empty()) {
            throw std::runtime_error("Columns list is empty");
        }
        FixedArray<float, 3> center(0);
        for (const auto& f : raw_frames) {
            center += f.at(columns_.begin()->joint_name)[0];
        }
        center /= raw_frames.size();
        for (auto& f : raw_frames) {
            f.at(columns_.begin()->joint_name)[0] -= center;
        }
    }
    if (std::isnan(frame_time_)) {
        throw std::runtime_error("Frame time not set");
    }
    transformed_frames_.resize(raw_frames.size());
    for (size_t i = 0; i < raw_frames.size(); ++i) {
        for (const auto& p : raw_frames[i]) {
            const FixedArray<float, 3>& position = p.second[0];
            const FixedArray<float, 3>& rotation = p.second[1];
            FixedArray<float, 4, 4> m = assemble_homogeneous_4x4(
                tait_bryan_angles_2_matrix(
                    rotation / 180.f * float(M_PI),
                    cfg_.rotation_order),
                position * cfg_.scale);
            const FixedArray<float, 4, 4>& n = cfg_.parameter_transformation;
            if (!transformed_frames_[i].insert({
                p.first,
                OffsetAndQuaternion<float>{dot2d(n, dot2d(m, n.T()))}}).second)
            {
                throw std::runtime_error("Could not insert transformed frame");
            }
        }
    }
}

const std::map<std::string, OffsetAndQuaternion<float>>& BvhLoader::get_frame(size_t id) const {
    if (id >= transformed_frames_.size()) {
        throw std::runtime_error("Frame index too large");
    }
    return transformed_frames_[id];
}

std::map<std::string, OffsetAndQuaternion<float>> BvhLoader::get_interpolated_frame(float seconds) const {
    if (transformed_frames_.empty()) {
        throw std::runtime_error("No frames to interpolate from");
    }
    float i = seconds / frame_time_;
    i = std::clamp(i, float{0}, float(transformed_frames_.size() - 1));
    size_t i0 = size_t(i);
    size_t i1 = i0 + 1;
    if (i1 > transformed_frames_.size()) {
        throw std::runtime_error("Frame interpolation internal error");
    }
    if (i1 == transformed_frames_.size()) {
        --i1;
        --i0;
    }
    float a0 = i - i0;
    const auto& f0 = get_frame(i0);
    const auto& f1 = get_frame(i1);
    std::map<std::string, OffsetAndQuaternion<float>> result;
    for (const auto& j0 : f0) {
        const auto& m0 = j0.second;
        const auto& m1 = f1.at(j0.first);
        if (!result.insert({j0.first, m0.slerp(m1, a0)}).second) {
            throw std::runtime_error("Could not insert interpolated frame");
        }
    }
    return result;
}
