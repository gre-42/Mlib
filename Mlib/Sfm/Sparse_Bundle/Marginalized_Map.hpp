#pragma once
#include <map>
#include <ostream>

namespace Mlib {

enum class MmState {
    ACTIVE,
    LINEARIZED,
    MARGINALIZED
};

inline std::ostream& operator << (std::ostream& ostr, MmState state) {
    switch(state) {
        case MmState::ACTIVE:
            ostr << "a";
            break;
        case MmState::LINEARIZED:
            ostr << "l";
            break;
        case MmState::MARGINALIZED:
            ostr << "m";
            break;
        default:
            throw std::runtime_error("Unknown MmState");
    }
    return ostr;
}

template <class TMapIterator, class TKey, class TValue>
class MarginalizedMapIteratorValue {
public:
    MarginalizedMapIteratorValue(TMapIterator& it, MmState state)
    : first(it->first),
      second(it->second),
      it_(it),
      state_(state) {}

    MarginalizedMapIteratorValue* operator -> () {
        // https://stackoverflow.com/a/20688236/2292832
        return this;
    }

    const MarginalizedMapIteratorValue* operator -> () const {
        // https://stackoverflow.com/a/20688236/2292832
        return this;
    }

    TKey& first;
    TValue& second;
    TMapIterator& it_;
    MmState state_;
};

template <class TMapIterator, class TKey, class TValue>
class MarginalizedMapIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = MarginalizedMapIteratorValue<TMapIterator, TKey, TValue>;
    using difference_type = std::ptrdiff_t;
    using pointer = MarginalizedMapIteratorValue<TMapIterator, TKey, TValue>*;
    using reference = MarginalizedMapIteratorValue<TMapIterator, TKey, TValue>&;

    MarginalizedMapIterator(
        const TMapIterator& active,
        const TMapIterator& linearized,
        const TMapIterator& marginalized,
        const TMapIterator& active_begin,
        const TMapIterator& linearized_begin,
        const TMapIterator& marginalized_begin,
        const TMapIterator& active_end,
        const TMapIterator& linearized_end,
        const TMapIterator& marginalized_end)
    : active_(active),
      linearized_(linearized),
      marginalized_(marginalized),
      active_begin_(active_begin),
      linearized_begin_(linearized_begin),
      marginalized_begin_(marginalized_begin),
      active_end_(active_end),
      linearized_end_(linearized_end),
      marginalized_end_(marginalized_end) {}

    bool operator != (const MarginalizedMapIterator& other) const {
        return (active_ != other.active_) || (linearized_ != other.linearized_) || (marginalized_ != other.marginalized_);
    }

    bool operator == (const MarginalizedMapIterator& other) const {
        return !(*this != other);
    }

    MarginalizedMapIterator& operator ++ () {
        if (marginalized_ != marginalized_end_) {
            ++marginalized_;
        } else if (linearized_ != linearized_end_) {
            ++linearized_;
        } else if (active_ != active_end_) {
            ++active_;
        } else {
            throw std::runtime_error("Iteration past the end");
        }
        return *this;
    }

    MarginalizedMapIterator& operator -- () {
        if (marginalized_ != marginalized_begin_) {
            --marginalized_;
        } else if (linearized_ != linearized_begin_) {
            --linearized_;
        } else if (active_ != active_begin_) {
            --active_;
        } else {
            throw std::runtime_error("Iteration past the beginning");
        }
        return *this;
    }

    auto operator * () {
        if (marginalized_ != marginalized_end_) {
            return MarginalizedMapIteratorValue<decltype((marginalized_)), decltype((marginalized_->first)), decltype((marginalized_->second))>(marginalized_, MmState::MARGINALIZED);
        } else if (linearized_ != linearized_end_) {
            return MarginalizedMapIteratorValue<decltype((linearized_)), decltype((linearized_->first)), decltype((linearized_->second))>(linearized_, MmState::LINEARIZED);
        } else if (active_ != active_end_) {
            return MarginalizedMapIteratorValue<decltype((active_)), decltype((active_->first)), decltype((active_->second))>(active_, MmState::ACTIVE);
        } else {
            throw std::runtime_error("Dereferenciation past the end");
        }
    }

    auto operator -> () {
        return **this;
    }

    auto operator * () const {
        if (marginalized_ != marginalized_end_) {
            return MarginalizedMapIteratorValue<decltype((marginalized_)), decltype((marginalized_->first)), decltype((marginalized_->second))>(marginalized_, MmState::MARGINALIZED);
        } else if (linearized_ != linearized_end_) {
            return MarginalizedMapIteratorValue<decltype((linearized_)), decltype((linearized_->first)), decltype((linearized_->second))>(linearized_, MmState::LINEARIZED);
        } else if (active_ != active_end_) {
            return MarginalizedMapIteratorValue<decltype((active_)), decltype((active_->first)), decltype((active_->second))>(active_, MmState::ACTIVE);
        } else {
            throw std::runtime_error("Dereferenciation past the end");
        }
    }

    auto operator -> () const {
        return **this;
    }

private:
    TMapIterator active_;
    TMapIterator linearized_;
    TMapIterator marginalized_;
    TMapIterator active_begin_;
    TMapIterator linearized_begin_;
    TMapIterator marginalized_begin_;
    TMapIterator active_end_;
    TMapIterator linearized_end_;
    TMapIterator marginalized_end_;
};

template <class TMapIterator, class TKey, class TValue>
class ReverseMarginalizedMapIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = MarginalizedMapIteratorValue<TMapIterator, TKey, TValue>;
    using difference_type = std::ptrdiff_t;
    using pointer = MarginalizedMapIteratorValue<TMapIterator, TKey, TValue>*;
    using reference = MarginalizedMapIteratorValue<TMapIterator, TKey, TValue>&;

    ReverseMarginalizedMapIterator(
        const TMapIterator& active,
        const TMapIterator& linearized,
        const TMapIterator& marginalized,
        const TMapIterator& active_begin,
        const TMapIterator& linearized_begin,
        const TMapIterator& marginalized_begin,
        const TMapIterator& active_end,
        const TMapIterator& linearized_end,
        const TMapIterator& marginalized_end)
    : active_(active),
      linearized_(linearized),
      marginalized_(marginalized),
      active_begin_(active_begin),
      linearized_begin_(linearized_begin),
      marginalized_begin_(marginalized_begin),
      active_end_(active_end),
      linearized_end_(linearized_end),
      marginalized_end_(marginalized_end) {}

    bool operator != (const ReverseMarginalizedMapIterator& other) const {
        return (active_ != other.active_) || (linearized_ != other.linearized_) || (marginalized_ != other.marginalized_);
    }

    bool operator == (const ReverseMarginalizedMapIterator& other) const {
        return !(*this != other);
    }

    ReverseMarginalizedMapIterator& operator ++ () {
        if (active_ != active_end_) {
            ++active_;
        } else if (linearized_ != linearized_end_) {
            ++linearized_;
        } else if (marginalized_ != marginalized_end_) {
            ++marginalized_;
        } else {
            throw std::runtime_error("Iteration past the end");
        }
        return *this;
    }

    ReverseMarginalizedMapIterator& operator -- () {
        if (active_ != active_begin_) {
            --active_;
        } else if (linearized_ != linearized_begin_) {
            --linearized_;
        } else if (marginalized_ != marginalized_begin_) {
            --marginalized_;
        } else {
            throw std::runtime_error("Iteration past the beginning");
        }
        return *this;
    }

    auto operator * () {
        if (active_ != active_end_) {
            return MarginalizedMapIteratorValue<decltype((active_)), decltype((active_->first)), decltype((active_->second))>(active_, MmState::ACTIVE);
        } else if (linearized_ != linearized_end_) {
            return MarginalizedMapIteratorValue<decltype((linearized_)), decltype((linearized_->first)), decltype((linearized_->second))>(linearized_, MmState::LINEARIZED);
        } else if (marginalized_ != marginalized_end_) {
            return MarginalizedMapIteratorValue<decltype((marginalized_)), decltype((marginalized_->first)), decltype((marginalized_->second))>(marginalized_, MmState::MARGINALIZED);
        } else {
            throw std::runtime_error("Dereferenciation past the end");
        }
    }

    auto operator -> () {
        return **this;
    }

    auto operator * () const {
        if (active_ != active_end_) {
            return MarginalizedMapIteratorValue<decltype((active_)), decltype((active_->first)), decltype((active_->second))>(active_, MmState::ACTIVE);
        } else if (linearized_ != linearized_end_) {
            return MarginalizedMapIteratorValue<decltype((linearized_)), decltype((linearized_->first)), decltype((linearized_->second))>(linearized_, MmState::LINEARIZED);
        } else if (marginalized_ != marginalized_end_) {
            return MarginalizedMapIteratorValue<decltype((marginalized_)), decltype((marginalized_->first)), decltype((marginalized_->second))>(marginalized_, MmState::MARGINALIZED);
        } else {
            throw std::runtime_error("Dereferenciation past the end");
        }
    }

    auto operator -> () const {
        return **this;
    }

private:
    TMapIterator active_;
    TMapIterator linearized_;
    TMapIterator marginalized_;
    TMapIterator active_begin_;
    TMapIterator linearized_begin_;
    TMapIterator marginalized_begin_;
    TMapIterator active_end_;
    TMapIterator linearized_end_;
    TMapIterator marginalized_end_;
};

template <class TMap>
class MarginalizedMap {
public:
    auto begin() {
        return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
            active_.begin(), linearized_.begin(), marginalized_.begin(),
            active_.begin(), linearized_.begin(), marginalized_.begin(),
            active_.end(), linearized_.end(), marginalized_.end());
    }
    auto end() {
        return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
            active_.end(), linearized_.end(), marginalized_.end(),
            active_.begin(), linearized_.begin(), marginalized_.begin(),
            active_.end(), linearized_.end(), marginalized_.end());
    }
    auto begin() const {
        return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
            active_.begin(), linearized_.begin(), marginalized_.begin(),
            active_.begin(), linearized_.begin(), marginalized_.begin(),
            active_.end(), linearized_.end(), marginalized_.end());
    }
    auto end() const {
        return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
            active_.end(), linearized_.end(), marginalized_.end(),
            active_.begin(), linearized_.begin(), marginalized_.begin(),
            active_.end(), linearized_.end(), marginalized_.end());
    }
    auto rbegin() {
        return ReverseMarginalizedMapIterator<decltype(active_.rbegin()), decltype(active_.rbegin()->first), decltype(active_.rbegin()->second)>(
            active_.rbegin(), linearized_.rbegin(), marginalized_.rbegin(),
            active_.rbegin(), linearized_.rbegin(), marginalized_.rbegin(),
            active_.rend(), linearized_.rend(), marginalized_.rend());
    }
    auto rend() {
        return ReverseMarginalizedMapIterator<decltype(active_.rbegin()), decltype(active_.rbegin()->first), decltype(active_.rbegin()->second)>(
            active_.rend(), linearized_.rend(), marginalized_.rend(),
            active_.rbegin(), linearized_.rbegin(), marginalized_.rbegin(),
            active_.rend(), linearized_.rend(), marginalized_.rend());
    }
    auto rbegin() const {
        return ReverseMarginalizedMapIterator<decltype(active_.rbegin()), decltype(active_.rbegin()->first), decltype(active_.rbegin()->second)>(
            active_.rbegin(), linearized_.rbegin(), marginalized_.rbegin(),
            active_.rbegin(), linearized_.rbegin(), marginalized_.rbegin(),
            active_.rend(), linearized_.rend(), marginalized_.rend());
    }
    auto rend() const {
        return ReverseMarginalizedMapIterator<decltype(active_.rbegin()), decltype(active_.rbegin()->first), decltype(active_.rbegin()->second)>(
            active_.rend(), linearized_.rend(), marginalized_.rend(),
            active_.rbegin(), linearized_.rbegin(), marginalized_.rbegin(),
            active_.rend(), linearized_.rend(), marginalized_.rend());
    }
    template <class TKey>
    auto find(const TKey& key) {
        {
            auto res_a = active_.find(key);
            if (res_a != active_.end()) {
                return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
                    res_a, linearized_.end(), marginalized_.end(),
                    active_.begin(), linearized_.begin(), marginalized_.begin(),
                    active_.end(), linearized_.end(), marginalized_.end());
            }
        }
        {
            auto res_b = linearized_.find(key);
            if (res_b != linearized_.end()) {
                return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
                    active_.begin(), res_b, marginalized_.end(),
                    active_.begin(), linearized_.begin(), marginalized_.begin(),
                    active_.end(), linearized_.end(), marginalized_.end());
            }
        }
        {
            auto res_m = marginalized_.find(key);
            if (res_m != marginalized_.end()) {
                return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
                    active_.begin(), linearized_.begin(), res_m,
                    active_.begin(), linearized_.begin(), marginalized_.begin(),
                    active_.end(), linearized_.end(), marginalized_.end());
            }
        }
        return end();
    }
    template <class TKey>
    auto find(const TKey& key) const {
        {
            auto res_a = active_.find(key);
            if (res_a != active_.end()) {
                return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
                    res_a, linearized_.end(), marginalized_.end(),
                    active_.begin(), linearized_.begin(), marginalized_.begin(),
                    active_.end(), linearized_.end(), marginalized_.end());
            }
        }
        {
            auto res_b = linearized_.find(key);
            if (res_b != linearized_.end()) {
                return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
                    active_.begin(), res_b, marginalized_.end(),
                    active_.begin(), linearized_.begin(), marginalized_.begin(),
                    active_.end(), linearized_.end(), marginalized_.end());
            }
        }
        {
            auto res_m = marginalized_.find(key);
            if (res_m != marginalized_.end()) {
                return MarginalizedMapIterator<decltype(active_.begin()), decltype(active_.begin()->first), decltype(active_.begin()->second)>(
                    active_.begin(), linearized_.begin(), res_m,
                    active_.begin(), linearized_.begin(), marginalized_.begin(),
                    active_.end(), linearized_.end(), marginalized_.end());
            }
        }
        return end();
    }
    template <class TKey>
    auto& at(const TKey& key) {
        auto res = find(key);
        if (res == end()) {
            throw std::runtime_error("Could not find key");
        }
        return res->second;
    }
    template <class TKey>
    auto& at(const TKey& key) const {
        auto res = find(key);
        if (res == end()) {
            throw std::runtime_error("Could not find key");
        }
        return res->second;
    }
    template <class TKeyValueTuple>
    void insert(const TKeyValueTuple& element) {
        active_.insert(element);
    }
    size_t size() const {
        return active_.size() + linearized_.size() + marginalized_.size();
    }
    template <class TIterator>
    void move_to_linearized(const TIterator& it) {
        auto nh = active_.extract(it);
        if (nh.empty()) {
            throw std::runtime_error("Move to linearized could not find node");
        }
        if (linearized_.find(it) != linearized_.end()) {
            throw std::runtime_error("Element already marked for marginalization");
        }
        linearized_.insert(std::move(nh));
    }
    template <class TIterator>
    void move_to_marginalized(const TIterator& it) {
        auto nh = linearized_.extract(it);
        if (nh.empty()) {
            throw std::runtime_error("Marginalize could not find node");
        }
        if (marginalized_.find(it) != marginalized_.end()) {
            throw std::runtime_error("Element already marginalized");
        }
        marginalized_.insert(std::move(nh));
    }
    void clear() {
        active_.clear();
        linearized_.clear();
        marginalized_.clear();
    }
    template <class TIterator>
    void erase(const TIterator& it) {
        if (find(it) == end()) {
            throw std::runtime_error("Could not erase element");
        }
        active_.erase(it);
        linearized_.erase(it);
        marginalized_.erase(it);
    }
    bool empty() const {
        return active_.empty() && linearized_.empty() && marginalized_.empty();
    }
    auto& operator [] (const typename TMap::key_type& k) {
        return active_[k];
    }
    std::map<typename TMap::key_type, const typename TMap::mapped_type*> sorted() const {
        std::map<typename TMap::key_type, const typename TMap::mapped_type*> res;
        for (const auto& p : *this) {
            if (res.find(p.first) != res.end()) {
                throw std::runtime_error("Duplicate key");
            }
            res.insert(std::make_pair(p.first, &p.second));
        }
        return res;
    }
    TMap active_;
    TMap linearized_;
    TMap marginalized_;
};

}
