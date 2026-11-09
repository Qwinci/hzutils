#pragma once

#include <stddef.h>

#include "type_traits.hpp"

namespace std {
#if defined(_MSC_VER) && !defined(__clang__)
#define __MOVE_ATTR [[msvc::intrinsic, nodiscard]]
#else
#define __MOVE_ATTR [[__gnu__::__artificial__, __gnu__::__always_inline__, nodiscard]] inline
#endif

	template<typename __T>
	__MOVE_ATTR constexpr remove_reference_t<__T>&& move(__T&& __value) noexcept {
		return static_cast<remove_reference_t<__T>&&>(__value);
	}

	template<typename __T>
	__MOVE_ATTR constexpr __T&& forward(remove_reference_t<__T>& __value) noexcept {
		return static_cast<__T&&>(__value);
	}

	template<typename __T>
	__MOVE_ATTR constexpr __T&& forward(remove_reference_t<__T>&& __value) noexcept {
		return static_cast<__T&&>(__value);
	}

#undef __MOVE_ATTR

	template<typename __T>
	constexpr void swap(__T& __a, __T& __b) {
		__T __tmp(std::move(__a));
		__a = std::move(__b);
		__b = std::move(__tmp);
	}

	template<typename __T, typename __U = __T>
	constexpr __T exchange(__T& __obj, __U&& __new_value) {
		__T __old = std::move(__obj);
		__obj = std::forward<__U>(__new_value);
		return __old;
	}

	template<typename __T>
	constexpr underlying_type_t<__T> to_underlying(__T __value) noexcept {
		return static_cast<underlying_type_t<__T>>(__value);
	}

	[[noreturn]] inline void unreachable() {
#if defined(_MSC_VER) && !defined(__clang__)
		__assume(false);
#else
		__builtin_unreachable();
#endif
	}

	struct in_place_t {
		explicit in_place_t() = default;
	};

	inline constexpr in_place_t in_place {};

	template<size_t __I>
	struct in_place_index_t {
		explicit in_place_index_t() = default;
	};

	template<size_t __I>
	inline constexpr in_place_index_t<__I> in_place_index {};

	template<typename __T, __T... __Ints>
	struct integer_sequence {
		using value_type = __T;

		static constexpr size_t size() noexcept {
			return sizeof...(__Ints);
		}
	};

	template<size_t... __Ints>
	using index_sequence = integer_sequence<size_t, __Ints...>;

#if hz_has_builtin(__make_integer_seq)
	template<typename __T, __T __N>
	using make_integer_sequence = __make_integer_seq<integer_sequence, __T, __N>;
#else
	template<typename __T, __T __N>
	using make_integer_sequence = integer_sequence<__T, __integer_pack(__N)...>;
#endif

	template<size_t __N>
	using make_index_sequence = make_integer_sequence<size_t, __N>;

	template<typename... __T>
	using index_sequence_for = make_index_sequence<sizeof...(__T)>;
}
