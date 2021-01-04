#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct ColumnDescription {
    std::string joint_name;
    size_t pose_index0;
    size_t pose_index1;
};

class BvhLoader {
public:
    explicit BvhLoader(const std::string& filename, bool demean, float scale);
    std::map<std::string, FixedArray<float, 4, 4>> get_frame(size_t id);
private:
    std::vector<std::map<std::string, FixedArray<float, 2, 3>>> frames_;
    std::map<std::string, FixedArray<float, 2, 3>> offsets_;
    std::list<ColumnDescription> columns_;
    float frame_time_;
    float scale_;
};

}
