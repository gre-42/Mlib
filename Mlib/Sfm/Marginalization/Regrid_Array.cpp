#include "Regrid_Array.hpp"

using namespace Mlib;

RegridArray::RegridArray(
    const std::map<UUID, size_t>& uuids_new,
    const std::map<size_t, UUID>& uuids_old)
: length_(uuids_new.size())
{
    // lerr() << "i " << ids_old;
    std::vector<size_t> ids_src_v;
    std::vector<size_t> ids_dst_v;
    ids_src_v.reserve(uuids_old.size());
    ids_dst_v.reserve(uuids_old.size());
    for (const auto& o : uuids_old) {
        auto it = uuids_new.find(o.second);
        if (it != uuids_new.end()) {
            ids_src_v.push_back(o.first);
            ids_dst_v.push_back(it->second);
        } else {
            // lerr() << "stripping index " << o.second << " " << o.first;
        }
    }
    ids_src_ = Array<size_t>(ids_src_v.data(), ids_src_v.data() + ids_src_v.size());
    ids_dst_ = Array<size_t>(ids_dst_v.data(), ids_dst_v.data() + ids_dst_v.size());
    // lerr() << "d " << ids_src_;
    // lerr() << "c " << ids_col_;
}

Array<float> RegridArray::regrid_1d(const Array<float>& a) {
    assert(a.ndim() == 1);
    Array<float> res = zeros<float>(ArrayShape{length_});
    for (size_t r = 0; r < ids_src_.length(); ++r) {
        res(ids_dst_(r)) = a(ids_src_(r));
    }
    return res;
}

Array<float> RegridArray::regrid_2d(const Array<float>& a) {
    assert(a.ndim() == 2);
    Array<float> res = zeros<float>(ArrayShape{length_, length_});
    for (size_t r = 0; r < ids_src_.length(); ++r) {
        for (size_t c = 0; c < ids_src_.length(); ++c) {
            res(ids_dst_(r), ids_dst_(c)) = a(ids_src_(r), ids_src_(c));
        }
    }
    return res;
}
