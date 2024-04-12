#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>

namespace Mlib {

template <class T>
class DestructionFunctionsTokensObject {
	DestructionFunctionsTokensObject(const DestructionFunctionsTokensObject&) = delete;
	DestructionFunctionsTokensObject& operator = (const DestructionFunctionsTokensObject&) = delete;
public:
	template <class TDerived>
	inline DestructionFunctionsTokensObject(const TDerived& o, SourceLocation loc)
		: object_{ o }
		, on_object_destroy_{ o->on_destroy, loc }
	{}
	inline T* operator -> () const {
		return object_.get();
	}
	inline T* get() const {
		return object_.get();
	}
	inline void on_destroy(std::function<void()> func, SourceLocation loc) {
		on_object_destroy_.add(std::move(func), std::move(loc));
	}
private:
	DanglingBaseClassPtr<T> object_;
	DestructionFunctionsRemovalTokens on_object_destroy_;
};

}
