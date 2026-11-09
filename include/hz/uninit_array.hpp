#pragma once

#include "config.hpp"

#include <stddef.h>

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	template<typename T, size_t N>
	struct uninit_array {
		constexpr T* data() {
			return storage.data;
		}

		static constexpr size_t size() {
			return N;
		}

	private:
		union storage {
			constexpr storage() {}

			constexpr ~storage() requires std::is_trivially_destructible_v<T> = default;
			constexpr ~storage() {}

			T data[N];
		} storage;
	};
}
