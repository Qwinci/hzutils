#pragma once

#include "config.hpp"

#include <stdarg.h>

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	template<typename T>
	T va_pop(va_list& list) {
		if constexpr (sizeof(T) < sizeof(int)) {
			return (T) va_arg(list, int);
		}
		else if constexpr (std::is_same_v<T, float>) {
			return static_cast<float>(va_arg(list, double));
		}
		else {
			return va_arg(list, T);
		}
	}
}
