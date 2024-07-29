#pragma once
#include "concepts.hpp"
#include "string_view.hpp"

namespace hz {
	namespace __detail {
		constexpr char to_lower(char c) {
			return static_cast<char>(c | 1 << 5);
		}

		static constexpr char CHARS[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	}

	template<typename T, typename C> requires(integral<T>)
	constexpr T to_integer(basic_string_view<C> str, int base, size_t* count_read) {
		bool sign = false;
		size_t offset = 0;
		if (str.starts_with(C {'-'})) {
			++offset;
			sign = true;
		}
		else if (str.starts_with(C {'+'})) {
			++offset;
		}

		if (offset == str.size()) {
			if (count_read) {
				*count_read = 0;
			}
			return T {};
		}

		if (base == 0) {
			if (offset + 1 < str.size() && str[offset] == C {'0'} &&
				(str[offset + 1] == C {'x'} || str[offset + 1] == C {'X'})) {
				base = 16;
				offset += 2;
			}
			else if (str[offset] == C {'0'}) {
				base = 8;
				++offset;
			}
			else {
				base = 10;
			}
		}
		else if (base == 8) {
			if (str[offset] == C {'0'}) {
				++offset;
			}
		}
		else if (base == 16) {
			if (offset + 1 < str.size() && str[offset] == C {'0'} &&
				(str[offset + 1] == C {'x'} || str[offset + 1] == C {'X'})) {
				offset += 2;
			}
		}

		if (offset == str.size()) {
			if (count_read) {
				*count_read = 0;
			}
			return T {};
		}

		T value {};
		for (size_t i = offset; i < str.size(); ++i) {
			auto c = str[i];
			if (c < C {'0'} || c > C {'z'}) {
				if (count_read) {
					*count_read = i;
				}
				if (sign) {
					value *= -1;
				}
				return value;
			}
			c = __detail::to_lower(c);
			if (c > __detail::CHARS[base - 1]) {
				if (count_read) {
					*count_read = i;
				}
				if (sign) {
					value *= -1;
				}
				return value;
			}
			value *= base;
			value += c <= '9' ? (c - '0') : (c - 'a' + 10);
		}

		if (count_read) {
			*count_read = str.size();
		}

		if (sign) {
			value *= -1;
		}
		return value;
	}
}
