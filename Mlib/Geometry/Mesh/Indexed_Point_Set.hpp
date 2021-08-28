#pragma once
#include <list>
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
    std::vector<double>& positions();
private:
    std::map<OrderableFixedArray<float, 2>, int> pts_;
    std::vector<double> positions_;
    int current_index_;
};

}
