#pragma once
#include <ostream>

namespace Mlib {

template <class TPosition, class TFlags>
struct PointAndFlags {
	using Point = TPosition;
	using value_type = TPosition::value_type;
	static consteval size_t length() {
		return TPosition::length();
	}
	PointAndFlags& operator = (const Point& other) {
		position = other;
		return *this;
	}
	operator const Point& () const {
		return position;
	}
	PointAndFlags operator * (const value_type& f) const {
		return {
			position * f,
			flags
		};
	}
	PointAndFlags operator + (const PointAndFlags& other) const {
		return {
			position + other.position,
			flags | other.flags
		};
	}
	TPosition operator - (const PointAndFlags& other) const {
		return position - other.position;
	}
	PointAndFlags& operator |= (const PointAndFlags& other) {
		flags |= other.flags;
		return *this;
	}
	template <class Archive>
	void serialize(Archive& archive) {
		archive(position);
		archive(flags);
	}
	TPosition position;
	TFlags flags;
};

template <class TPosition, class TFlags>
std::ostream& operator << (std::ostream& ostr, const PointAndFlags<TPosition, TFlags>& p) {
	return (ostr << '(' << p.position << ", " << p.flags << ')');
}

}