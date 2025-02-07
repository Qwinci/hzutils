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
		static_assert(sizeof(m) == sizeof(uintptr_t));

		uintptr_t offset;
		__builtin_memcpy(&offset, &m, sizeof(uintptr_t));

		return std::launder(reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) - offset));
	}
}
