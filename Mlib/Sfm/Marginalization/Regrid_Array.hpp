#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <map>

namespace Mlib {

class RegridArray {
public:
    RegridArray(
        const std::map<UUID, size_t>& uuids_new,
        const std::map<size_t, UUID>& uuids_old);

    Array<float> regrid_1d(const Array<float>& a);
    Array<float> regrid_2d(const Array<float>& a);

private:
    Array<size_t> ids_src_;
    Array<size_t> ids_dst_;
    size_t length_;
};

}
