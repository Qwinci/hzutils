#pragma once

#include <stdint.h>

#include "config.hpp"
#include "string_view.hpp"
#include "string_utils.hpp"

#if HZ_FREESTANDING
#include "concepts.hpp"
#include "type_traits.hpp"
#else
#include <concepts>
#include <type_traits>
#endif

namespace hz::formatting {
	template<typename T, typename C>
	concept format_sink = requires (T& sink, hz::basic_string_view<C> str) {
		sink.append(str);
	};

	struct format_int_opts {
		int precision {-1};
		int min_width {-1};
		unsigned int base {10};
		bool always_sign : 1 {};
		bool plus_as_space : 1 {};
		bool right_pad : 1 {};
		bool prefix : 1 {};
		bool upper : 1 {};
		bool sign : 1 {};
		bool zero_pad : 1 {};
	};

	namespace __detail {
		template<typename C>
		struct base_chars;
		template<>
		struct base_chars<char> {
			static constexpr char CHARS[] = "0123456789abcdef";
			static constexpr char UPPER_CHARS[] = "0123456789ABCDEF";
		};
		template<>
		struct base_chars<wchar_t> {
			static constexpr wchar_t CHARS[] = L"0123456789abcdef";
			static constexpr wchar_t UPPER_CHARS[] = L"0123456789ABCDEF";
		};
		template<>
		struct base_chars<char8_t> {
			static constexpr char8_t CHARS[] = u8"0123456789abcdef";
			static constexpr char8_t UPPER_CHARS[] = u8"0123456789ABCDEF";
		};
		template<>
		struct base_chars<char16_t> {
			static constexpr char16_t CHARS[] = u"0123456789abcdef";
			static constexpr char16_t UPPER_CHARS[] = u"0123456789ABCDEF";
		};
		template<>
		struct base_chars<char32_t> {
			static constexpr char32_t CHARS[] = U"0123456789abcdef";
			static constexpr char32_t UPPER_CHARS[] = U"0123456789ABCDEF";
		};
	}

	template<typename C, typename T, format_sink<C> Sink>
	constexpr int format_int(Sink& sink, T value, const format_int_opts& opts) {
		constexpr int max_digits = sizeof(T) * 8;

		C buf[max_digits];
		C* ptr = buf + max_digits;

		if (value != 0 || opts.precision != 0) {
			const C* chars = opts.upper ? __detail::base_chars<C>::UPPER_CHARS : __detail::base_chars<C>::CHARS;

			do {
				*--ptr = chars[value % opts.base];
				value /= opts.base;
			} while (value);
		}

		int value_width = static_cast<int>(buf + max_digits - ptr);
		int actual_width = value_width < opts.precision ? opts.precision : value_width;

		if (opts.always_sign || opts.plus_as_space || opts.sign) {
			++actual_width;
		}

		if (value_width) {
			if ((opts.base == 2 || opts.base == 16) && opts.prefix) {
				actual_width += 2;
			}
			else if (opts.base == 8 && opts.prefix) {
				++actual_width;
			}
		}

		bool zero_pad = opts.zero_pad && opts.precision < 0;

		if (!opts.right_pad && !zero_pad) {
			C pad_c = C(' ');
			for (int i = actual_width; i < opts.min_width; ++i) {
				sink.append({&pad_c, 1});
			}
		}

		if (opts.sign) {
			C sign = C('-');
			sink.append({&sign, 1});
		}
		else if (opts.always_sign) {
			C sign = C('+');
			sink.append({&sign, 1});
		}
		else if (opts.plus_as_space) {
			C sign = C(' ');
			sink.append({&sign, 1});
		}

		if (opts.prefix && value_width) {
			if (opts.base == 2) {
				C prefix[2] {C('0'), C(opts.upper ? 'B' : 'b')};
				sink.append({prefix, 2});
			}
			else if (opts.base == 8) {
				C prefix = C('0');
				sink.append({&prefix, 1});
			}
			else if (opts.base == 16) {
				C prefix[2] {C('0'), C(opts.upper ? 'X' : 'x')};
				sink.append({prefix, 2});
			}
		}

		if (!opts.right_pad && zero_pad) {
			C pad_c = C('0');
			for (int i = actual_width; i < opts.min_width; ++i) {
				sink.append({&pad_c, 1});
			}
		}

		C zero = C('0');
		for (int i = value_width; i < opts.precision; ++i) {
			sink.append({&zero, 1});
		}

		sink.append({ptr, static_cast<size_t>(value_width)});

		if (opts.right_pad) {
			C pad_c = C(' ');
			for (int i = actual_width; i < opts.min_width; ++i) {
				sink.append({&pad_c, 1});
			}
		}

		return actual_width >= opts.min_width ? actual_width : opts.min_width;
	}

	template<typename T>
	struct bin_fmt {
		T value;
	};

	template<typename T>
	struct oct_fmt {
		T value;
	};

	template<typename T>
	struct hex_fmt {
		constexpr explicit hex_fmt(T value) : value {value} {}
		explicit hex_fmt(const void* value) : value {static_cast<T>(reinterpret_cast<uintptr_t>(value))} {}

		T value;
	};

	struct min_width {
		int value = -1;
	};

	struct precision {
		int value = -1;
	};

	struct space_pad {};
	struct zero_pad {};
	struct left_pad {};
	struct right_pad {};

	template<typename C, format_sink<C> Sink>
	class formatter {
	public:
		constexpr explicit formatter(Sink& sink) : sink {sink} {}

		constexpr formatter& operator<<(basic_string_view<C> str) {
			int width = static_cast<int>(str.size());
			if (!opts.right_pad) {
				C pad_c = C(' ');
				for (int i = width; i < opts.min_width; ++i) {
					sink.append({&pad_c, 1});
				}
			}

			sink.append(str);

			if (opts.right_pad) {
				C pad_c = C(' ');
				for (int i = width; i < opts.min_width; ++i) {
					sink.append({&pad_c, 1});
				}
			}

			return *this;
		}

		constexpr formatter& set_precision(int precision) {
			opts.precision = precision;
			return *this;
		}

		constexpr formatter& set_min_width(int min_width) {
			opts.min_width = min_width;
			return *this;
		}

		constexpr formatter& set_int_base(unsigned int base) {
			opts.base = base;
			return *this;
		}

		constexpr formatter& set_always_sign(bool value) {
			opts.always_sign = value;
			return *this;
		}

		constexpr formatter& set_plus_as_space(bool value) {
			opts.plus_as_space = value;
			return *this;
		}

		constexpr formatter& set_right_pad(bool value) {
			opts.right_pad = value;
			return *this;
		}

		constexpr formatter& set_int_prefix(bool prefix) {
			opts.prefix = prefix;
			return *this;
		}

		constexpr formatter& set_int_upper(bool value) {
			opts.upper = value;
			return *this;
		}

		constexpr formatter& set_zero_pad(bool value) {
			opts.zero_pad = value;
			return *this;
		}

		constexpr formatter& reset() {
			opts = {};
			opts.prefix = true;
			return *this;
		}

		template<typename T>
		constexpr formatter& operator<<(bin_fmt<T> value) {
			opts.base = 2;
			operator<<(value.value);
			opts.base = 10;
			return *this;
		}

		template<typename T>
		constexpr formatter& operator<<(oct_fmt<T> value) {
			opts.base = 8;
			operator<<(value.value);
			opts.base = 10;
			return *this;
		}

		template<typename T>
		constexpr formatter& operator<<(hex_fmt<T> value) {
			opts.base = 16;
			operator<<(value.value);
			opts.base = 10;
			return *this;
		}

		constexpr formatter& operator<<(min_width width) {
			opts.min_width = width.value;
			return *this;
		}

		constexpr formatter& operator<<(precision prec) {
			opts.precision = prec.value;
			return *this;
		}

		constexpr formatter& operator<<(space_pad) {
			opts.zero_pad = false;
			return *this;
		}

		constexpr formatter& operator<<(zero_pad) {
			opts.zero_pad = true;
			return *this;
		}

		constexpr formatter& operator<<(left_pad) {
			opts.right_pad = false;
			return *this;
		}

		constexpr formatter& operator<<(right_pad) {
			opts.right_pad = true;
			return *this;
		}

		template<typename T> requires (std::is_pointer_v<T> && !std::is_same_v<T, const C*>)
		constexpr formatter& operator<<(T value) {
			return operator<<(hex_fmt {reinterpret_cast<uintptr_t>(value)});
		}

		constexpr formatter& operator<<(const C* value) {
			return operator<<(basic_string_view<C> {value});
		}

		constexpr formatter& operator<<(bool value) {
			using namespace literals;

			if constexpr (std::is_same_v<C, char>) {
				return operator<<(value ? "true"_sv : "false"_sv);
			}
			else if constexpr (std::is_same_v<C, wchar_t>) {
				return operator<<(value ? L"true"_sv : L"false"_sv);
			}
			else if constexpr (std::is_same_v<C, char8_t>) {
				return operator<<(value ? u8"true"_sv : u8"false"_sv);
			}
			else if constexpr (std::is_same_v<C, char16_t>) {
				return operator<<(value ? u"true"_sv : u"false"_sv);
			}
			else if constexpr (std::is_same_v<C, char32_t>) {
				return operator<<(value ? U"true"_sv : U"false"_sv);
			}
			else {
				static_assert(false);
			}
		}

		constexpr formatter& operator<<(signed char value) {
			unsigned int actual_value;
			if (value < 0) {
				opts.sign = true;
				actual_value = static_cast<unsigned int>(-value);
			}
			else {
				opts.sign = false;
				actual_value = static_cast<unsigned char>(value);
			}

			format_int<C>(sink, actual_value, opts);
			return *this;
		}

		constexpr formatter& operator<<(unsigned char value) {
			opts.sign = false;
			format_int<C, unsigned int>(sink, value, opts);
			return *this;
		}

		constexpr formatter& operator<<(short value) {
			unsigned int actual_value;
			if (value < 0) {
				opts.sign = true;
				actual_value = static_cast<unsigned int>(-value);
			}
			else {
				opts.sign = false;
				actual_value = static_cast<unsigned short>(value);
			}

			format_int<C>(sink, actual_value, opts);
			return *this;
		}

		constexpr formatter& operator<<(unsigned short value) {
			opts.sign = false;
			format_int<C, unsigned int>(sink, value, opts);
			return *this;
		}

		constexpr formatter& operator<<(int value) {
			unsigned int actual_value;
			if (value < 0) {
				opts.sign = true;
				actual_value = 0U - static_cast<unsigned int>(value);
			}
			else {
				opts.sign = false;
				actual_value = static_cast<unsigned int>(value);
			}

			format_int<C>(sink, actual_value, opts);
			return *this;
		}

		constexpr formatter& operator<<(unsigned int value) {
			opts.sign = false;
			format_int<C>(sink, value, opts);
			return *this;
		}

		constexpr formatter& operator<<(long value) {
			unsigned long actual_value;
			if (value < 0) {
				opts.sign = true;
				actual_value = 0UL - static_cast<unsigned long>(value);
			}
			else {
				opts.sign = false;
				actual_value = static_cast<unsigned long>(value);
			}

			format_int<C>(sink, actual_value, opts);
			return *this;
		}

		constexpr formatter& operator<<(unsigned long value) {
			opts.sign = false;
			format_int<C>(sink, value, opts);
			return *this;
		}

		constexpr formatter& operator<<(long long value) {
			unsigned long long actual_value;
			if (value < 0) {
				opts.sign = true;
				actual_value = 0ULL - static_cast<unsigned long long>(value);
			}
			else {
				opts.sign = false;
				actual_value = static_cast<unsigned long long>(value);
			}

			format_int<C>(sink, actual_value, opts);
			return *this;
		}

		constexpr formatter& operator<<(unsigned long long value) {
			opts.sign = false;
			format_int<C>(sink, value, opts);
			return *this;
		}

	private:
		Sink& sink;
		format_int_opts opts {.prefix = true};
	};

	template<typename C, format_sink<C> Sink, typename... Args>
	constexpr void format_to(Sink& sink, Args&&... args) {
		formatter<C, Sink> fmt {sink};
		((fmt << args), ...);
	}
}
