#pragma once

#include "config.hpp"

#include <stdint.h>

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	template<typename T, typename B>
	B* container_of(T* ptr, std::remove_const_t<T> B::*field) {
#ifdef _MSC_VER
		static_assert(sizeof(field) == 4, "only non-virtual bases are supported in container_of");
#endif

		if constexpr (sizeof(field) == 4) {
			return __builtin_launder(reinterpret_cast<B*>(reinterpret_cast<uintptr_t>(ptr) - __builtin_bit_cast(uint32_t, field)));
		}
		else {
			return __builtin_launder(reinterpret_cast<B*>(reinterpret_cast<uintptr_t>(ptr) - __builtin_bit_cast(uint64_t, field)));
		}
	}
}
