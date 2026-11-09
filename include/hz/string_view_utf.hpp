#pragma once

#include "string_view.hpp"
#include "utf.hpp"

namespace hz {
	template<typename T>
	constexpr codepoint_iterator<T> basic_string_view<T>::codepoints() const {
		return {data_, size_};
	}

	template<typename T>
	constexpr auto basic_string_view<T>::utf8_encode() const {
		return utf8_encode_iterator<typename codepoint_iterator<T>::inner> {codepoints()};
	}

	template<typename T>
	constexpr auto basic_string_view<T>::utf16_encode() const {
		return utf16_encode_iterator<typename codepoint_iterator<T>::inner> {codepoints()};
	}

	template<typename T>
	constexpr codepoint_iterator<T> basic_string_view<T>::utf32_encode() const {
		return codepoints();
	}
}
