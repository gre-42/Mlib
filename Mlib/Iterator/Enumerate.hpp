#pragma once
#include <iterator>
#include <utility>

namespace Mlib {

// From: https://stackoverflow.com/questions/11328264/python-like-loop-enumeration-in-c
template<typename Iterable>
class enumerate_named_object
{
private:
    static Iterable container();
    std::size_t size_;
    decltype(std::begin(container())) begin_;
    decltype(std::end(container())) end_;

public:
    struct end_type {};
    enumerate_named_object(Iterable& iter)
        : size_(0)
        , begin_(std::begin(iter))
        , end_(std::end(iter))
    {}

    const enumerate_named_object& begin() const { return *this; }
    end_type end() const { return {}; }

    bool operator != (const end_type&) const
    {
        return begin_ != end_;
    }

    void operator++()
    {
        ++begin_;
        ++size_;
    }

    std::pair<std::size_t, decltype(*begin_)> operator*() const {
        return { size_, *begin_ };
    }
};

template<typename Iterable>
class enumerate_temporary_object
{
private:
    Iterable container_;
    std::size_t size_;
    decltype(std::begin(container_)) begin_;
    decltype(std::end(container_)) end_;

public:
    struct end_type {};
    enumerate_temporary_object(enumerate_temporary_object&& other) noexcept
        : enumerate_temporary_object{ std::move(other.container_) }
    {}
    enumerate_temporary_object(Iterable&& iter)
        : container_(std::move(iter))
        , size_(0)
        , begin_(std::begin(container_))
        , end_(std::end(container_))
    {}

    enumerate_temporary_object begin() { return std::move(*this); }
    end_type end() const { return {}; }

    bool operator != (const end_type&) const
    {
        return begin_ != end_;
    }

    void operator++()
    {
        ++begin_;
        ++size_;
    }

    std::pair<std::size_t, decltype(*begin_)> operator*() const {
        return { size_, *begin_ };
    }
};

template<typename Iterable>
auto enumerate(Iterable&& iter) -> enumerate_temporary_object<Iterable>
{
    return { std::move(iter) };
}

template<typename Iterable>
auto enumerate(Iterable& iter) -> enumerate_named_object<Iterable&>
{
    return { iter };
}

template<typename Iterable>
auto enumerate(const Iterable& iter) -> enumerate_named_object<const Iterable&>
{
    return { iter };
}

}
