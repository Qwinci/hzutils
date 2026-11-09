#pragma once

#include "config.hpp"

#include <stddef.h>

#if defined(_MSC_VER) && !defined(__clang__)

#if HZ_FREESTANDING
#undef memcpy
#undef memset
extern "C" void* memcpy(void* __restrict dest, const void* __restrict src, size_t size) noexcept;
extern "C" void* memmove(void* dest, const void* src, size_t size) noexcept;
extern "C" void* memset(void* __restrict dest, int ch, size_t size) noexcept;
#else
#include <string.h>
#endif

#endif

namespace hz {
	template<typename T>
	T unaligned_load(const void* ptr) {
		T value;
#if defined(_MSC_VER) && !defined(__clang__)
		memcpy(&value, ptr, sizeof(T));
#else
		__builtin_memcpy(&value, ptr, sizeof(T));
#endif
		return value;
	}

	template<typename T>
	void unaligned_store(void* ptr, T value) {
#if defined(_MSC_VER) && !defined(__clang__)
		memcpy(ptr, &value, sizeof(T));
#else
		__builtin_memcpy(ptr, &value, sizeof(T));
#endif
	}

	template<typename T>
	void memory_copy(T* __restrict dest, const T* __restrict src, size_t count) {
#if defined(_MSC_VER) && !defined(__clang__)
		memcpy(dest, src, sizeof(T) * count);
#else
		__builtin_memcpy(dest, src, sizeof(T) * count);
#endif
	}

	template<>
	inline void memory_copy<void>(void* __restrict dest, const void* __restrict src, size_t count) {
#if defined(_MSC_VER) && !defined(__clang__)
		memcpy(dest, src, count);
#else
		__builtin_memcpy(dest, src, count);
#endif
	}

	template<typename T>
	void memory_set(T* __restrict dest, int ch, size_t count) {
#if defined(_MSC_VER) && !defined(__clang__)
		memset(dest, ch, sizeof(T) * count);
#else
		__builtin_memset(dest, ch, sizeof(T) * count);
#endif
	}

	template<>
	inline void memory_set<void>(void* __restrict dest, int ch, size_t count) {
#if defined(_MSC_VER) && !defined(__clang__)
		memset(dest, ch, count);
#else
		__builtin_memset(dest, ch, count);
#endif
	}

	template<typename T>
	void memory_move(T* dest, const T* src, size_t count) {
#if defined(_MSC_VER) && !defined(__clang__)
		memmove(dest, src, sizeof(T) * count);
#else
		__builtin_memmove(dest, src, sizeof(T) * count);
#endif
	}

	template<>
	inline void memory_move<void>(void* dest, const void* src, size_t count) {
#if defined(_MSC_VER) && !defined(__clang__)
		memmove(dest, src, count);
#else
		__builtin_memmove(dest, src, count);
#endif
	}
}
