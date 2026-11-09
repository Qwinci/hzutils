#pragma once

#include "config.hpp"
#include "formatting.hpp"
#include "string_view.hpp"

#include <stdarg.h>

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz::__detail {
	template<typename Sink, typename C>
	concept fallible_sink = requires(Sink& sink, basic_string_view<C> str) {
		static_cast<bool>(sink.append(str));
	};

	template<typename C, typename Sink>
	struct fallible_sink_adapter {
		constexpr void append(basic_string_view<C> str) {
			if (!failed && !static_cast<bool>(sink.append(str))) {
				failed = true;
			}
		}

		Sink& sink;
		bool failed = false;
	};

	template<typename C, typename Sink>
	using printf_sink = std::conditional_t<fallible_sink<Sink, C>, fallible_sink_adapter<C, Sink>, Sink>;
}

namespace hz {
	enum class format_width {
		none,
		char_,
		short_,
		long_,
		longlong,
		intmax,
		size,
		longdouble
	};

	template<typename C>
	struct printf_info {
		formatting::format_int_opts opts {};
		format_width width {};
		const C* fmt_ptr {};
	};

	// Handles a specific printf format specifier returning how many chars it produced
	// or a negative value on error.
	// Note: The handler must increment fmt_ptr by how many chars it consumed.
	// Note: The handler should be templated over the sink type to be able to support adapters.
	// Note: fmt_ptr may point to a null byte, the handler must not consume it.
	template<typename T, typename C, typename Sink>
	concept printf_handler = requires(T& self, Sink& sink, printf_info<C>& info, va_list& ap) {
		{ self.handle_format(sink, info, ap) } -> std::same_as<int>;
	};

	template<typename C, formatting::format_sink<C> Sink, printf_handler<C, __detail::printf_sink<C, Sink>> Handler>
	int printf_impl(Sink& sink, Handler& handler, const C* fmt, va_list ap);
}

namespace hz::__detail {
	template<typename C, formatting::format_sink<C> Sink, printf_handler<C, Sink> Handler, typename Failed>
	int printf_impl(Sink& sink, Handler& handler, const C* fmt, va_list va, Failed&& failed) {
		va_list ap;
		va_copy(ap, va);

		int written = 0;

		while (true) {
			const C* start = fmt;

			for (; *fmt && *fmt != C('%'); ++fmt);
			int len = static_cast<int>(fmt - start);
			for (; fmt[0] == C('%') && fmt[1] == C('%'); fmt += 2) ++len;

			if (len) {
				written += len;
				sink.append({start, static_cast<size_t>(len)});

				if (failed()) {
					va_end(ap);
					return -1;
				}

				continue;
			}

			if (!*fmt) {
				break;
			}

			++fmt;

			printf_info<C> info {};

			while (true) {
				switch (*fmt) {
					case C('-'):
						++fmt;
						info.opts.right_pad = true;
						continue;
					case C('0'):
						++fmt;
						info.opts.zero_pad = true;
						continue;
					case C('+'):
						++fmt;
						info.opts.always_sign = true;
						continue;
					case C(' '):
						++fmt;
						info.opts.plus_as_space = true;
						continue;
					case C('#'):
						++fmt;
						info.opts.prefix = true;
						continue;
					default:
						break;
				}

				break;
			}

			C ch = *fmt;
			if (is_digit(ch)) {
				info.opts.min_width = 0;
				do {
					info.opts.min_width *= 10;
					info.opts.min_width += ch - C('0');
					ch = *++fmt;
				} while (is_digit(ch));
			}
			else if (ch == C('*')) {
				++fmt;
				info.opts.min_width = va_arg(ap, int);
				if (info.opts.min_width < 0) {
					info.opts.right_pad = true;
					info.opts.min_width = -info.opts.min_width;
				}
			}

			if (*fmt == C('.')) {
				++fmt;
				ch = *fmt;
				if (is_digit(ch)) {
					info.opts.precision = 0;
					do {
						info.opts.precision *= 10;
						info.opts.precision += ch - C('0');
						ch = *++fmt;
					} while (is_digit(ch));
				}
				else if (ch == C('*')) {
					++fmt;
					info.opts.precision = va_arg(ap, int);
				}
				else {
					info.opts.precision = 0;
				}
			}

			switch (*fmt) {
				case C('h'):
					++fmt;
					if (*fmt == C('h')) {
						++fmt;
						info.width = format_width::char_;
					}
					else {
						info.width = format_width::short_;
					}
					break;
				case C('l'):
					++fmt;
					if (*fmt == C('l')) {
						++fmt;
						info.width = format_width::longlong;
					}
					else {
						info.width = format_width::long_;
					}
					break;
				case C('j'):
					++fmt;
					info.width = format_width::intmax;
					break;
				case C('z'):
				case C('t'):
					++fmt;
					info.width = format_width::size;
					break;
				case C('L'):
					++fmt;
					info.width = format_width::longdouble;
					break;
				default:
					break;
			}

			if (info.opts.right_pad) {
				info.opts.zero_pad = false;
			}
			if (info.opts.plus_as_space && info.opts.always_sign) {
				info.opts.plus_as_space = false;
			}

			info.fmt_ptr = fmt;

			int res = handler.handle_format(sink, info, ap);
			if (res < 0) {
				va_end(ap);
				return res;
			}

			if (failed()) {
				va_end(ap);
				return -1;
			}

			written += res;
			fmt = info.fmt_ptr;
		}

		va_end(ap);

		return written;
	}
}

namespace hz {
	template<typename C, formatting::format_sink<C> Sink, printf_handler<C, __detail::printf_sink<C, Sink>> Handler>
	int printf_impl(Sink& sink, Handler& handler, const C* fmt, va_list ap) {
		if constexpr (__detail::fallible_sink<Sink, C>) {
			__detail::fallible_sink_adapter<C, Sink> adapter {sink};
			return __detail::printf_impl(adapter, handler, fmt, ap, [&] {
				return adapter.failed;
			});
		}
		else {
			return __detail::printf_impl(sink, handler, fmt, ap, [] {
				return false;
			});
		}
	}
}
