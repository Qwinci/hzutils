#pragma once

#include <stddef.h>

namespace std {
	template<typename __T>
	class initializer_list {
	public:
		using value_type = __T;
		using reference = const __T&;
		using const_reference = const __T&;
		using size_type = size_t;
		using iterator = const __T*;
		using const_iterator = const __T*;

		constexpr initializer_list() noexcept = default;

		[[nodiscard]] constexpr size_type size() const noexcept {
			return __size;
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return __size == 0;
		}

		constexpr const __T* begin() const noexcept {
			return __data;
		}

		constexpr const __T* end() const noexcept {
			return __data + __size;
		}

		constexpr const __T* data() const noexcept {
			return __data;
		}

	private:
		const __T* __data {};
		size_t __size {};
	};
}
