#pragma once
#include <stdint.h>
#if __STDC_HOSTED__ == 1
#include <new>
#else
#include "new.hpp"
#endif

namespace hz {
	template<typename T, typename M>
	T* container_of(M* ptr, M T::* m) {
		if constexpr (sizeof(m) == sizeof(uintptr_t)) {
			uintptr_t offset = 0;
			__builtin_memcpy(&offset, &m, sizeof(uintptr_t));
			return std::launder(reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) - offset));
		}
		else {
			static_assert(sizeof(m) == 4);
			uint32_t offset = 0;
			__builtin_memcpy(&offset, &m, 4);
			return std::launder(reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) - offset));
		}
	}
}
