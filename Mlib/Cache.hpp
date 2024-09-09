#pragma once
#include <cstddef>
#include <vector>

namespace Mlib {

template <class TElement>
class Cache {
public:
	using Element = TElement;
	explicit Cache(size_t nelements) {
		elements_.reserve(nelements);
	}
	template <class TMatches, class TCreate>
	TElement& compute(const TMatches& matches, const TCreate& create) {
		for (auto it = elements_.begin(); it != elements_.end(); ++it) {
			if (matches(*it)) {
				std::rotate(it, it + 1, elements_.end());
				return elements_.back();
			}
		}
		if (elements_.size() == elements_.capacity()) {
			elements_.erase(elements_.begin());
		}
		return elements_.emplace_back(create());
	}
private:
	std::vector<TElement> elements_;
};

}
