#pragma once

#include <stddef.h>

namespace hz {
	template<typename T>
	constexpr size_t const_strlen(const T* str) {
		size_t len = 0;
		while (*str++) ++len;
		return len;
	}

	template<typename C>
	constexpr C to_lower_ascii(C ch) {
		return ch >= C('A') && ch <= C('Z') ? (static_cast<C>(ch | 1 << 5)) : ch;
	}

	template<typename C>
	constexpr C to_upper_ascii(C ch) {
		return ch >= C('a') && ch <= C('z') ? (static_cast<C>(ch & ~(1 << 5))) : ch;
	}

	template<typename C>
	constexpr bool is_digit(C ch) {
		return ch >= C('0') && ch <= C('9');
	}

	template<typename C>
	constexpr bool is_xdigit(C ch) {
		if (ch >= C('0') && ch <= C('9')) {
			return true;
		}
		ch = to_lower_ascii(ch);
		return ch >= C('a') && ch <= C('f');
	}

	template<typename C>
	constexpr bool is_space(C ch) {
		return ch >= 0 && ch <= C(' ');
	}
}
