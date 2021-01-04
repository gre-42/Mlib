#include "Load_Bvh.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/String.hpp>
#include <fstream>
#include <regex>

using namespace Mlib;

BvhLoader::BvhLoader(const std::string& filename, bool center) {
    std::ifstream f{filename};
    if (f.fail()) {
        throw std::runtime_error("Could not open " + filename);
    }
    std::string line;
    std::string joint_name;
    bool in_data_section = false;
    while (std::getline(f, line)) {
        if (!in_data_section) {
            static const std::regex name_re{"^[\\s\\n\\r]*(?:ROOT|JOINT)\\s+(\\w+)[\\s\\n\\r]*$"};
            static const std::regex chan_re{"^[\\s\\n\\r]*CHANNELS\\s+(\\d+)\\s+(.+)[\\n\\r]*$"};
            static const std::regex motion_re{"^[\\s\\n\\r]*MOTION[\\s\\n\\r]*$"};
            std::smatch match;
            if (std::regex_match(line, match, name_re)) {
                joint_name = match[1].str();
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
                    } else if (c == "Zrotation") {
                        cid0 = 1;
                        cid1 = 0;
                    } else if (c == "Yrotation") {
                        cid0 = 1;
                        cid1 = 1;
                    } else if (c == "Xrotation") {
                        cid0 = 1;
                        cid1 = 2;
                    } else {
                        throw std::runtime_error("Unknown channel type");
                    }
                    columns_.push_back(ColumnDescription{.joint_name = joint_name, .pose_index0 = cid0, .pose_index1 = cid1});
                }
            } else if (std::regex_match(line, match, motion_re)) {
                static const std::regex frames_re{"^\\s*Frames:	(\\d+)\\s*"};
                static const std::regex frame_time_re{"^\\s*Frame Time:\\s*(\\S+)\\s*"};
                if (!std::getline(f, line)) {
                    throw std::runtime_error("Could not get line");
                }
                if (!std::regex_match(line, match, frames_re)) {
                    throw std::runtime_error("Could not match frames: " + line);
                }
                size_t nframes = safe_stoz(match[1].str());
                frames_.reserve(nframes);
                if (!std::getline(f, line)) {
                    throw std::runtime_error("Could not get line");
                }
                if (!std::regex_match(line, match, frame_time_re)) {
                    throw std::runtime_error("Could not match frame time: " + line);
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
            frames_.emplace_back();
            size_t i = 0;
            for (const auto& c : columns_) {
                frames_.back()[c.joint_name](c.pose_index0, c.pose_index1) = d[i++];
            }
        }
    }
    if (center) {
        if (columns_.empty()) {
            throw std::runtime_error("Columns list is empty");
        }
        FixedArray<float, 3> center(0);
        for (const auto& f : frames_) {
            center += f.at(columns_.begin()->joint_name)[0];
        }
        center /= frames_.size();
        for (auto& f : frames_) {
            f.at(columns_.begin()->joint_name)[0] -= center;
        }
    }
}

std::map<std::string, FixedArray<float, 4, 4>> BvhLoader::get_frame(size_t id) {
    if (id >= frames_.size()) {
        throw std::runtime_error("Frame ID too large");
    }
    std::map<std::string, FixedArray<float, 4, 4>> result;
    for (const auto& p : frames_[id]) {
        FixedArray<float, 3> position = p.second[0];
        FixedArray<float, 3> rotation = p.second[1];
        // https://research.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/BVH.html
        // v*R = v*YXZ
        result[p.first] = assemble_homogeneous_4x4(tait_bryan_angles_2_matrix(rotation, {1, 0, 2}), position);
    }
    return result;
}
