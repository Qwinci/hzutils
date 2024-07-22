#pragma once
#include "concepts.hpp"
#include "string_view.hpp"

namespace hz {
	template<typename T, typename C> requires(integral<T>)
	constexpr T to_integer(basic_string_view<C> str, size_t& count_read) {
		T value {};
		for (size_t i = 0; i < str.size(); ++i) {
			auto c = str[i];
			if (c < C {'0'} || c > C {'9'}) {
				count_read = i;
				return value;
			}
			value *= 10;
			value += c - C {'0'};
		}

		count_read = str.size();
		return value;
	}
}
