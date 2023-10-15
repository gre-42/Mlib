#pragma once
#include <utility>

namespace Mlib {

// From: https://stackoverflow.com/questions/11328264/python-like-loop-enumeration-in-c
template<typename Iterable>
class enumerate_object
{
    private:
        Iterable iter_;
        std::size_t size_;
        decltype(std::begin(iter_)) begin_;
        const decltype(std::end(iter_)) end_;

    public:
        enumerate_object(Iterable iter):
            iter_(iter),
            size_(0),
            begin_(std::begin(iter)),
            end_(std::end(iter))
        {}

        const enumerate_object& begin() const { return *this; }
        const enumerate_object& end()   const { return *this; }

        bool operator!=(const enumerate_object&) const
        {
            return begin_ != end_;
        }

        void operator++()
        {
            ++begin_;
            ++size_;
        }

        auto operator*() const
            -> std::pair<std::size_t, decltype(*begin_)&>
        {
            return { size_, *begin_ };
        }
};

template<typename Iterable>
auto enumerate(Iterable&& iter)
    -> enumerate_object<Iterable>
{
    return { std::forward<Iterable>(iter) };
}

}
