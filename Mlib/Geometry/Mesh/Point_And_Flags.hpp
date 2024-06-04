#pragma once
#include <concepts>
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

template <class TPosition, class TFlags, std::floating_point TRhs>
auto operator * (
	const PointAndFlags<TPosition, TFlags>& lhs,
	const TRhs& f)
{
	using V = typename TPosition::value_type;
	using TResultV = decltype(V() * TRhs());
	using TResult = decltype(TPosition().template casted<TResultV>());
	return PointAndFlags<TResult, TFlags>{
		lhs.position.template casted<TResultV>() * (TResultV)f,
		lhs.flags
	};
}

template <std::floating_point TLhs, class TPosition, class TFlags>
auto operator * (
	const TLhs& f,
	const PointAndFlags<TPosition, TFlags>& rhs)
{
	using V = typename TPosition::value_type;
	using TResultV = decltype(TLhs() * V());
	using TResult = decltype(TPosition().template casted<TResultV>());
	return PointAndFlags<TResult, TFlags>{
		(TResultV)f * rhs.position.template casted<TResultV>(),
		rhs.flags
	};
}

template <class TPosition, class TFlags>
std::ostream& operator << (std::ostream& ostr, const PointAndFlags<TPosition, TFlags>& p) {
	return (ostr << '(' << p.position << ", " << p.flags << ')');
}

}
