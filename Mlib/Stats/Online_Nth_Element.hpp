#pragma once
#include <iosfwd>
#include <queue>

namespace Mlib {

template <class TData, class TScore>
struct DataAndScore {
    TData data;
    TScore score;
    bool operator < (const DataAndScore& other) const {
        return score < other.score;
    }
    bool operator > (const DataAndScore& other) const {
        return score > other.score;
    }
    friend std::ostream& operator << (std::ostream& ostr, const DataAndScore& element) {
        return ostr << '(' << element.data << ", " << element.score << ')';
    }
};

template <class TData, class TScore, template<class> class TCompare>
class OnlineNthElement:
    private std::priority_queue<
        DataAndScore<TData, TScore>,
        std::vector<DataAndScore<TData, TScore>>,
        TCompare<typename std::vector<DataAndScore<TData, TScore>>::value_type>>
{
    using Q = std::priority_queue<
        DataAndScore<TData, TScore>,
        std::vector<DataAndScore<TData, TScore>>,
        TCompare<typename std::vector<DataAndScore<TData, TScore>>::value_type>>;
public:
    explicit OnlineNthElement(size_t max_size)
        : max_size_{ max_size }
    {
        Q::c.reserve(max_size);
    }
    void insert(const TScore& score, const TData& data) {
        Q& queue = *this;
        if (queue.size() < max_size_) {
            // do nothing
        } else if (TCompare<TScore>()(score, queue.top().score)) {
            queue.pop();
        } else {
            return;
        }
        queue.emplace(data, score);
    }
    size_t size() const {
        const Q& queue = *this;
        return queue.size();
    }
    const DataAndScore<TData, TScore>& nth() const {
        const Q& queue = *this;
        return queue.top();
    }
private:
    size_t max_size_;
};

template <class TData, class TScore>
using OnlineNthSmallest = OnlineNthElement<TData, TScore, std::less>;

template <class TData, class TScore>
using OnlineNthLargest = OnlineNthElement<TData, TScore, std::greater>;

}
