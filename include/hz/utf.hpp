#pragma once

#include "config.hpp"

#include <stddef.h>

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	inline constexpr char32_t utf_error_short = static_cast<char32_t>(-1);
	inline constexpr char32_t utf_error_invalid = static_cast<char32_t>(-2);

	template<typename T>
	class codepoint_iterator {
	public:
		class inner {
		public:
			constexpr char32_t operator*() const {
				return current;
			}

			constexpr bool operator==(const inner& other) const {
				return data == other.data;
			}

			constexpr inner& operator++() {
				++data;
				decode_current();
				return *this;
			}

		private:
			friend codepoint_iterator;

			constexpr void decode_current() {
				if (data == end) {
					return;
				}

				T first = *data;

				if constexpr (std::is_same_v<T, char> || std::is_same_v<T, char8_t>) {
					if (!(first & 0x80)) {
						current = static_cast<char32_t>(static_cast<unsigned char>(first));
					}
					else if ((first & 0xE0) == 0xC0) {
						if (data + 1 == end) {
							current = utf_error_short;
							return;
						}

						T second = data[1];
						if ((second & 0xC0) == 0x80) {
							++data;
							current = static_cast<char32_t>(
								(static_cast<unsigned char>(second) & 0x3F) |
								(static_cast<unsigned char>(first) & 0x1F) << 6);
						}
						else {
							current = utf_error_invalid;
						}

						if (current < 0x80) {
							current = utf_error_invalid;
						}
					}
					else if ((first & 0xF0) == 0xE0) {
						if (data + 1 == end) {
							current = utf_error_short;
							return;
						}

						T second = data[1];
						if ((second & 0xC0) != 0x80) {
							current = utf_error_invalid;
							return;
						}

						++data;

						if (data + 1 == end) {
							current = utf_error_short;
							return;
						}

						T third = data[1];
						if ((third & 0xC0) == 0x80) {
							++data;
							current = static_cast<char32_t>(
								(static_cast<unsigned char>(third) & 0x3F) |
								(static_cast<unsigned char>(second) & 0x3F) << 6 |
								(static_cast<unsigned char>(first) & 0xF) << 12);
						}
						else {
							current = utf_error_invalid;
						}

						if (current < 0x800) {
							current = utf_error_invalid;
						}
					}
					else if ((first & 0xF8) == 0xF0) {
						if (data + 1 == end) {
							current = utf_error_short;
							return;
						}

						T second = data[1];
						if ((second & 0xC0) != 0x80) {
							current = utf_error_invalid;
							return;
						}

						++data;

						if (data + 1 == end) {
							current = utf_error_short;
							return;
						}

						T third = data[1];
						if ((third & 0xC0) != 0x80) {
							current = utf_error_invalid;
							return;
						}

						++data;

						if (data + 1 == end) {
							current = utf_error_short;
							return;
						}

						T fourth = data[1];
						if ((fourth & 0xC0) == 0x80) {
							++data;
							current = static_cast<char32_t>(
								(static_cast<unsigned char>(fourth) & 0x3F) |
								(static_cast<unsigned char>(third) & 0x3F) << 6 |
								(static_cast<unsigned char>(second) & 0x3F) << 12 |
								(static_cast<unsigned char>(first) & 7) << 18);
						}
						else {
							current = utf_error_invalid;
						}

						if (current < 0x10000 || current > 0x10FFFF) {
							current = utf_error_invalid;
						}
					}
					else {
						current = utf_error_invalid;
					}
				}
				else if constexpr (std::is_same_v<T, char16_t> || (std::is_same_v<T, wchar_t> && sizeof(wchar_t) == 2)) {
					if (first < 0xD800 || first > 0xDBFF || (data + 1) == end) {
						current = first;
					}
					else {
						T second = data[1];
						if (second >= 0xDC00 && second <= 0xDFFF) {
							++data;
							current = 0x10000 + (first - 0xD800) * 0x400 + (second - 0xDC00);
						}
						else {
							current = first;
						}
					}
				}
				else {
					current = static_cast<char32_t>(*data);
				}
			}

			constexpr explicit inner(const T* end) : data {end} {}
			constexpr inner(const T* data, const T* end) : data {data}, end {end} {
				decode_current();
			}

			const T* data {};
			const T* end {};
			char32_t current {};
	};

	constexpr codepoint_iterator(const T* data, size_t size) : data {data}, end_ {data + size} {}

	[[nodiscard]] constexpr inner begin() const {
		return {data, end_};
	}

	[[nodiscard]] constexpr inner end() const {
		return inner {end_};
	}

	private:
		const T* data;
		const T* end_;
	};

	struct utf8_encode_result {
		unsigned char chars[4];
		int len;
	};

	constexpr utf8_encode_result utf8_encode(char32_t cp) {
		utf8_encode_result res {};

		if (cp < 0x80) {
			res.chars[0] = static_cast<unsigned char>(cp);
			res.len = 1;
		}
		else if (cp < 0x800) {
			res.chars[0] = static_cast<unsigned char>((cp >> 6) | 0xC0);
			res.chars[1] = static_cast<unsigned char>((cp & 0x3F) | 0x80);
			res.len = 2;
		}
		else if (cp < 0x10000) {
			res.chars[0] = static_cast<unsigned char>((cp >> 12) | 0xE0);
			res.chars[1] = static_cast<unsigned char>(((cp >> 6) & 0x3F) | 0x80);
			res.chars[2] = static_cast<unsigned char>((cp & 0x3F) | 0x80);
			res.len = 3;
		}
		else if (cp < 0x110000) {
			res.chars[0] = static_cast<unsigned char>((cp >> 18) | 0xF0);
			res.chars[1] = static_cast<unsigned char>(((cp >> 12) & 0x3F) | 0x80);
			res.chars[2] = static_cast<unsigned char>(((cp >> 6) & 0x3F) | 0x80);
			res.chars[3] = static_cast<unsigned char>((cp & 0x3F) | 0x80);
			res.len = 4;
		}

		return res;
	}

	struct utf16_encode_result {
		char16_t chars[2];
		int len;
	};

	constexpr utf16_encode_result utf16_encode(char32_t cp) {
		utf16_encode_result res {};

		if (cp < 0x10000) {
			res.chars[0] = static_cast<char16_t>(cp);
			res.len = 1;
		}
		else if (cp < 0x110000) {
			cp -= 0x10000;
			res.chars[0] = static_cast<char16_t>(cp / 0x400 + 0xD800);
			res.chars[1] = static_cast<char16_t>(cp % 0x400 + 0xDC00);
			res.len = 2;
		}

		return res;
	}

	template<typename It>
	class utf8_encode_iterator {
	public:
		class inner {
		public:
			constexpr utf8_encode_result operator*() const {
				return utf8_encode(*it);
			}

			constexpr bool operator==(const inner& other) const {
				return it == other.it;
			}

			constexpr inner& operator++() {
				++it;
				return *this;
			}

		private:
			friend utf8_encode_iterator;

			constexpr explicit inner(It it) : it {it} {}

			It it;
		};

		constexpr utf8_encode_iterator(It first, It last) : first {first}, last {last} {}

		template<typename U>
		constexpr utf8_encode_iterator(const U& it) : first {it.begin()}, last {it.end()} {}

		[[nodiscard]] constexpr inner begin() const {
			return inner {first};
		}

		[[nodiscard]] constexpr inner end() const {
			return inner {last};
		}

	private:
		It first;
		It last;
	};

	template<typename It>
	class utf16_encode_iterator {
	public:
		class inner {
		public:
			constexpr utf16_encode_result operator*() const {
				return utf16_encode(*it);
			}

			constexpr bool operator==(const inner& other) const {
				return it == other.it;
			}

			constexpr inner& operator++() {
				++it;
				return *this;
			}

		private:
			friend utf16_encode_iterator;

			constexpr explicit inner(It it) : it {it} {}

			It it;
		};

		constexpr utf16_encode_iterator(It first, It last) : first {first}, last {last} {}

		template<typename U>
		constexpr utf16_encode_iterator(const U& it) : first {it.begin()}, last {it.end()} {}

		[[nodiscard]] constexpr inner begin() const {
			return inner {first};
		}

		[[nodiscard]] constexpr inner end() const {
			return inner {last};
		}

	private:
		It first;
		It last;
	};
}
