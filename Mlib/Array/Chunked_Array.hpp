#pragma once
#include <Mlib/Os/Os.hpp>
#include <optional>
#include <vector>

namespace Mlib {

template <class TContainer>
class ChunkedArray;

template <class TOuterIterator>
class UnNestedIterator {
public:
    using difference_type = std::size_t;
    using value_type = std::remove_reference_t<decltype(*TOuterIterator()->begin())>;
    UnNestedIterator(
        const TOuterIterator& outer_begin,
        const TOuterIterator& outer_end,
        std::size_t index)
        : outer_{ outer_begin }
        , outer_end_{ outer_end }
        , inner_{ outer_begin == outer_end
            ? std::nullopt
            : std::optional{ outer_begin->begin() } }
        , index_{ index }
    {}
    ~UnNestedIterator() = default;
    UnNestedIterator& operator ++ () {
        ++(*inner_);
        if (*inner_ == outer_->end()) {
            ++outer_;
            if (outer_ != outer_end_) {
                inner_ = outer_->begin();
            }
        }
        ++index_;
        return *this;
    }
    std::size_t operator - (const UnNestedIterator& rhs) const {
        return index_ - rhs.index_;
    }
    bool operator == (const UnNestedIterator& other) const {
        if (other.inner_.has_value()) {
            verbose_abort("Expected right hand side to be the end iterator");
        }
        return outer_ == other.outer_;
    }
    bool operator != (const UnNestedIterator& other) const {
        return !(*this == other);
    }
    value_type& operator * () const {
        return **inner_;
    }
    value_type& operator -> () const {
        return **inner_;
    }
private:
    TOuterIterator outer_;
    TOuterIterator outer_end_;
    std::optional<decltype(TOuterIterator()->begin())> inner_;
    std::size_t index_;
};

template <class TContainer>
class ChunkedArray {
public:
    using value_type = TContainer::value_type::value_type;
    explicit ChunkedArray(std::size_t chunk_size)
        : chunk_size_{ chunk_size }
    {}
    ~ChunkedArray() = default;
    template <class... Args>
    inline value_type& emplace_back(Args... args) {
        auto& last = (container_.empty() || (container_.back().size() == container_.back().capacity()))
            ? create_new_chunk()
            : container_.back();
        return last.emplace_back(std::forward<Args>(args)...);
    }
    auto begin() {
        return UnNestedIterator{ container_.begin(), container_.end(), 0 };
    }
    auto end() {
        return UnNestedIterator{ container_.end(), container_.end(), size() };
    }
    auto begin() const {
        return UnNestedIterator{ container_.begin(), container_.end(), 0 };
    }
    auto end() const {
        return UnNestedIterator{ container_.end(), container_.end(), size() };
    }
    bool empty() const {
        return container_.empty();
    }
    std::size_t size() const {
        if (container_.empty()) {
            return 0;
        }
        return (container_.size() - 1) * chunk_size_ + container_.back().size();
    }
    template <class TResult = std::vector<value_type>>
    auto to_vector() const {
        // return TResult(begin(), end());
        TResult result;
        result.reserve(size());
        for (const auto& v : *this) {
            result.emplace_back(v);
        }
        return result;
    };
private:
    auto& create_new_chunk() {
        auto& res = container_.emplace_back();
        res.reserve(chunk_size_);
        return res;
    }
    TContainer container_;
    std::size_t chunk_size_;
};

}
