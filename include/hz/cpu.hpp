#pragma once

#include "config.hpp"

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace hz {
#if defined(_M_AMD64) || defined(_M_IX86) || defined(__x86_64__) || defined(__i386__)
#define HZ_CPU_X86 1

	HZ_FORCEINLINE void spin_hint() {
#if defined(_MSC_VER) && !defined(__clang__)
		_mm_pause();
#else
		__builtin_ia32_pause();
#endif
	}

#elif defined(_M_ARM64) || defined(__aarch64__)
#define HZ_CPU_AARCH64 1

	HZ_FORCEINLINE void spin_hint() {
#if defined(_MSC_VER) && !defined(__clang__)
		__isb(_ARM64_BARRIER_SY);
#else
		asm volatile("isb");
#endif
	}

	HZ_FORCEINLINE void wait_for_event() {
#if defined(_MSC_VER) && !defined(__clang__)
		__wfe();
#else
		asm volatile("wfe");
#endif
	}

#if !defined(_MSC_VER) || defined(__clang__)
#define HZ_CPU_HAS_ARMED_LOAD 1

	template<typename T>
	HZ_FORCEINLINE T armed_load_relaxed(const T* ptr) {
		T value;
		if constexpr (sizeof(T) == 1) {
			asm volatile("ldxrb %w0, %1" : "=r"(value) : "Q"(*ptr));
		}
		else if constexpr (sizeof(T) == 2) {
			asm volatile("ldxrh %w0, %1" : "=r"(value) : "Q"(*ptr));
		}
		else if constexpr (sizeof(T) == 4) {
			asm volatile("ldxr %w0, %1" : "=r"(value) : "Q"(*ptr));
		}
		else {
			asm volatile("ldxr %0, %1" : "=r"(value) : "Q"(*ptr));
		}

		return value;
	}

#endif
#endif
}
