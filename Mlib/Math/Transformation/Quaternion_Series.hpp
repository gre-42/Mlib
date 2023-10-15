#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Positive_Modulo.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

struct Interpolator {
	size_t i0;
	size_t i1;
	float alpha;
};

template <class TDir, class TPos, size_t length>
class QuaternionSeries {
public:
	QuaternionSeries()
		: last_{ SIZE_MAX }
	{}
	QuaternionSeries(
		const OffsetAndQuaternion<TDir, TPos>& qo,
		std::chrono::steady_clock::time_point time)
		: QuaternionSeries()
	{
		append(qo, time);
	}

	void append(const OffsetAndQuaternion<TDir, TPos>& qo, std::chrono::steady_clock::time_point time) {
		last_ = (last_ + 1) % length;
		data_(last_) = qo;
		times_(last_) = time;
	}

	OffsetAndQuaternion<TDir, TPos> get(std::chrono::steady_clock::time_point time) const {
		return get(interpolator(time));
	}

	OffsetAndQuaternion<TDir, TPos> get(const Interpolator& interp) const {
		return data_(interp.i0).slerp(data_(interp.i1), interp.alpha);
	}

	Interpolator interpolator(std::chrono::steady_clock::time_point time) const {
		if (last_ == SIZE_MAX) {
			THROW_OR_ABORT("QuaternionSeries::interpolator called on empty sequence");
		}
		if (time == std::chrono::steady_clock::time_point()) {
			THROW_OR_ABORT("QuaternionSeries::interpolator received uninitialized time");
		}
		for (size_t i = 0; i < length; ++i) {
			size_t j0 = (size_t)positive_modulo((int)last_ - (int)i - 1, length);
			size_t j1 = (size_t)positive_modulo((int)last_ - (int)i, length);
			if (times_(j1) == std::chrono::steady_clock::time_point()) {
				verbose_abort("QuaternionSeries::interpolator internal error (0)");
			}
			if (times_(j0) == std::chrono::steady_clock::time_point()) {
				return Interpolator{
					.i0 = j1,
					.i1 = j1,
					.alpha = 0.f
				};
			}
			if (times_(j0) <= time) {
				if (times_(j1) <= time) {
					return Interpolator{
						.i0 = last_,
						.i1 = last_,
						.alpha = 0.f
					};
				}
				auto d = times_(j1) - times_(j0);
				auto alpha = float((time - times_(j0)).count() / double(d.count()));
				return Interpolator{
					.i0 = j0,
					.i1 = j1,
					.alpha = alpha
				};
			} else {
				return Interpolator{
					.i0 = j0,
					.i1 = j0,
					.alpha = 0.f
				};
			}
		}
		verbose_abort("QuaternionSeries::interpolator internal error (1)");
	}
private:
	FixedArray<OffsetAndQuaternion<TDir, TPos>, length> data_;
	FixedArray<std::chrono::steady_clock::time_point, length> times_;
	size_t last_;
};

}
