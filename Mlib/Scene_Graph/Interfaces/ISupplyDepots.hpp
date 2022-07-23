#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class ISupplyDepots {
public:
    virtual void add_supply_depot(
        const FixedArray<double, 3>& position,
        const std::map<std::string, uint32_t>& supply_depot) = 0;
};

}
