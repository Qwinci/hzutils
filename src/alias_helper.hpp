#pragma once

#include "../include/hz/config.hpp"

namespace hz {
	template<typename T>
	HZ_FORCEINLINE T aliased_load(const unsigned char* ptr) {
#if !defined(_MSC_VER) || defined(__clang__)
		T value;
		__builtin_memcpy(&value, ptr, sizeof(T));
		return value;
#else
		return *reinterpret_cast<const T*>(ptr);
#endif
	}

	template<typename T, typename U = T>
	HZ_FORCEINLINE void aliased_store(unsigned char* ptr, U value) {
#if !defined(_MSC_VER) || defined(__clang__)
		__builtin_memcpy(ptr, &value, sizeof(T));
#else
		*reinterpret_cast<T*>(ptr) = static_cast<T>(value);
#endif
	}
}
