#pragma once
#include <stddef.h>

constexpr void* operator new(size_t, void* ptr) noexcept {
	return ptr;
}

namespace std {
	template<typename __T>
	constexpr __T* launder(__T* __ptr) {
		return __builtin_launder(__ptr);
	}
}
