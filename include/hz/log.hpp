#pragma once

#include "config.hpp"
#include "formatting.hpp"
#include "memory_utils.hpp"

#if HZ_FREESTANDING
#include "utility.hpp"
#else
#include <utility>
#endif

namespace hz {
	struct endlog_type {
		explicit endlog_type() = default;
	};

	inline constexpr endlog_type endlog {};

	template<typename C, formatting::format_sink<C> Sink, size_t MaxMsgSize = 256>
	class log {
	public:
		constexpr explicit log(Sink& sink) : sink {sink} {}

		log(const log&) = delete;
		log& operator=(const log&) = delete;

		template<typename T>
		log& operator<<(T&& value) {
			fmt << value;
			return *this;
		}

		log& operator<<(endlog_type) {
			sink.append({buf.data, static_cast<size_t>(buf.ptr - buf.data)});
			buf.ptr = buf.data;
			fmt.reset();
			return *this;
		}

		log& append(basic_string_view<C> str) {
			buf.append(str);
			return *this;
		}

		constexpr void reset() {
			buf.ptr = buf.data;
		}

	private:
		struct buffer {
			constexpr buffer() = default;

			void append(basic_string_view<C> str) {
				size_t space_remaining = MaxMsgSize - static_cast<size_t>(ptr - data);
				size_t count = str.size() < space_remaining ? str.size() : space_remaining;

				memory_copy(ptr, str.data(), count);
				ptr += count;
			}

			C data[MaxMsgSize] {};
			C* ptr = data;
		};

		buffer buf;
		formatting::formatter<C, buffer> fmt {buf};
		Sink& sink;
	};

	template<formatting::format_sink<char> Sink>
	log(Sink& sink) -> log<char, Sink>;
}
