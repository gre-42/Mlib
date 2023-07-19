#include "Pack_Boxes.hpp"

using namespace Mlib;

using NamedSize = std::pair<const std::string*, const FixedArray<int, 2>&>;

std::list<std::list<NameAndBoxPosition>> Mlib::pack_boxes(
    const std::map<std::string, FixedArray<int, 2>>& box_sizes,
    const FixedArray<int, 2>& container_size)
{
    if (box_sizes.empty()) {
        return {};
    }
    std::list<NamedSize> sorted;
    for (const auto& [n, b] : box_sizes) {
        sorted.push_back({&n, b});
    }
    sorted.sort([](const NamedSize& a, const NamedSize& b){return a.second(1) > b.second(1);});
    std::list<std::list<NameAndBoxPosition>> result;
    result.emplace_back();
    int left = 0;
    int bottom = 0;
    int height = -1;
    while (!sorted.empty()) {
        const auto& [name, size] = sorted.front();
        sorted.pop_front();
        if (height == -1) {
            height = size(1);
        }
        if (size(0) > container_size(0)) {
            THROW_OR_ABORT("Width of box is too large");
        }
        if (size(1) > container_size(1)) {
            THROW_OR_ABORT("Height of box is too large");
        }
        if (left + size(0) >= container_size(0)) {
            left = 0;
            bottom += height;
            height = -1;
            if (bottom + size(1) > container_size(1)) {
                bottom = 0;
                result.emplace_back();
            }
        }
        result.back().emplace_back(NameAndBoxPosition{
            .name = *name,
            .bottom_left = {left, bottom}});
        left += size(0);
    }
    return result;
}
