#pragma once
#include <Mlib/Array/Chunked_Array.hpp>
#include <Mlib/Cache.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <list>
#include <vector>

namespace Mlib {

template <class TAxisAlignedBoundingBox, class TPair>
class BvhCacheElement {
public:
	TAxisAlignedBoundingBox aabb = uninitialized;
	ChunkedArray<std::list<std::vector<const TPair*>>> data{ 100 };
};

template <class TCache, class TBvh>
class CachingBvh {
public:
	using Cache = TCache;
	CachingBvh(const TBvh& bvh, TCache cache)
		: bvh_{ bvh }
		, cache_{ std::move(cache) }
	{}
	CachingBvh(const TBvh& bvh, size_t cache_size)
		: CachingBvh{ bvh, TCache{ cache_size } }
	{}
	template <class TAxisAlignedBoundingBox, class TVisitor>
	bool visit(
		const TAxisAlignedBoundingBox& large_aabb,
		const TAxisAlignedBoundingBox& small_aabb,
		const TVisitor& visitor) const
	{
		auto& el = cache_.compute(
			[&](const auto& e) {
				return e.aabb.contains(small_aabb);
			},
			[&]() {
				typename TCache::Element e;
				e.aabb = large_aabb;
				bvh_.visit_pairs(large_aabb, [&](const auto& d) {
					e.data.emplace_back(&d);
					return true;
					});
				return e;
			});
		for (const auto* d : el.data) {
			if (small_aabb.intersects(d->first)) {
				if (!visitor(d->second)) {
					return false;
				}
			}
		}
		return true;
	}
private:
	const TBvh& bvh_;
	mutable TCache cache_;
};

template <class TData, class TPayload, size_t tndim>
using CachingAabbBvh =
	CachingBvh<
		Cache<
			BvhCacheElement<
				AxisAlignedBoundingBox<TData, tndim>,
				std::pair<AxisAlignedBoundingBox<TData, tndim>, TPayload>
			>
		>,
		Bvh<TData, TPayload, tndim>>;

}
