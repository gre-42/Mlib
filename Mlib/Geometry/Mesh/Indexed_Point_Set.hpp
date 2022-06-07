#pragma once
#include <cstddef>
#include <map>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;

class IndexedPointSet {
public:
    IndexedPointSet();
    ~IndexedPointSet();
    int operator () (float x, float y);
    bool exists(float x, float y);
    int next_index();
    std::vector<double>& positions();
private:
    std::map<OrderableFixedArray<float, 2>, int> pts_;
    std::vector<double> positions_;
    int next_index_;
};

}
