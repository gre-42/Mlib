#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <cstdint>

namespace Mlib {

class ITextureHandle {
public:
	using handle_type = uint64_t;
	inline ITextureHandle()
		: handle_{ (handle_type)-1 }
	{}
	virtual ~ITextureHandle() = default;
	template <class T>
	inline const T& handle() const {
		return const_cast<ITextureHandle*>(this)->handle();
	}
	template <class T>
	inline T& handle() {
		static_assert(sizeof(T) <= sizeof(handle_type));
		return reinterpret_cast<T&>(handle_);
	}
private:
	handle_type handle_;
};

}
