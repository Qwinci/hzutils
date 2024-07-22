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

		[[nodiscard]] constexpr size_t size() const noexcept {
			return __size;
		}

		constexpr const_iterator begin() const noexcept {
			return __start;
		};

		constexpr const_iterator end() const noexcept {
			return __start + __size;
		}

	private:
		const __T* __start {};
		size_t __size {};
	};
}
