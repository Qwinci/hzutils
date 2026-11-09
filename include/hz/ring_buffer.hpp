#pragma once

#include "config.hpp"
#include "math.hpp"
#include "span.hpp"

#include <stddef.h>

#if HZ_FREESTANDING
#include "memory.hpp"
#include "type_traits.hpp"
#else
#include <memory>
#include <type_traits>
#endif

extern "C" void* memcpy(void* __restrict dest, const void* __restrict src, size_t size);

namespace hz {
	namespace __detail {
		template<typename T>
		void copy(T* dest, const T* src, size_t count) {
#if defined(_MSC_VER) && !defined(__clang__)
			memcpy(dest, src, count * sizeof(T));
#else
			__builtin_memcpy(dest, src, count * sizeof(T));
#endif
		}
	}

	template<typename T>
	class ring_buffer {
		static constexpr bool is_copyable = std::is_trivially_copyable_v<T>;
		static constexpr bool is_destructible = std::is_trivially_destructible_v<T>;

	public:
		constexpr ring_buffer() = default;

		constexpr ring_buffer(T* data, size_t size) : data_ {data}, size_ {size} {
			size_mask = size_ - 1;
		}

		constexpr ring_buffer(span<T> data) : data_ {data.data()}, size_ {data.size()} {
			size_mask = size_ - 1;
		}

		ring_buffer(const ring_buffer&) = delete;
		ring_buffer& operator=(const ring_buffer&) = delete;

		constexpr ring_buffer(ring_buffer&& other) noexcept
			: data_ {other.data_}, size_ {other.size_}, write_pos {other.write_pos}, read_pos {other.read_pos} {
			size_mask = size_ - 1;
			other.data_ = nullptr;
			other.size_ = 0;
			other.write_pos = 0;
			other.read_pos = 0;
		}

		constexpr ring_buffer& operator=(ring_buffer&& other) noexcept {
			clear();
			data_ = other.data_;
			size_ = other.size_;
			size_mask = size_ - 1;
			write_pos = other.write_pos;
			read_pos = other.read_pos;
			other.data_ = nullptr;
			other.size_ = 0;
			other.write_pos = 0;
			other.read_pos = 0;
			return *this;
		}

		[[nodiscard]] constexpr size_t size() const {
			return write_pos - read_pos;
		}

		[[nodiscard]] constexpr size_t space_available() const {
			return size_ - size();
		}

		constexpr void write(span<const T> data) {
			if constexpr (is_copyable) {
				size_t pos = write_pos & size_mask;
				size_t at_pos = hz::min(size_ - pos, data.size());
				__detail::copy(&data_[pos], data.data(), at_pos);

				if (at_pos != data.size()) {
					size_t remainder = data.size() - at_pos;
					__detail::copy(data_, data.data() + at_pos, remainder);
				}
			}
			else {
				for (size_t i = 0; i < data.size(); ++i) {
					std::construct_at(&data_[(write_pos + i) & size_mask], data[i]);
				}
			}

			write_pos += data.size();
		}

		constexpr void write(const T& value) {
			size_t pos = write_pos & size_mask;

			if constexpr (is_copyable) {
				__detail::copy(&data_[pos], &value, 1);
			}
			else {
				std::construct_at(&data_[pos], value);
			}

			++write_pos;
		}

		constexpr void write(T&& value) {
			size_t pos = write_pos & size_mask;

			if constexpr (is_copyable) {
				__detail::copy(&data_[pos], &value, 1);
			}
			else {
				std::construct_at(&data_[pos], std::move(value));
			}

			++write_pos;
		}

		constexpr void read(span<T> data) {
			if constexpr (is_copyable && is_destructible) {
				size_t pos = read_pos & size_mask;
				size_t at_pos = hz::min(size_ - pos, data.size());
				__detail::copy(data.data(), &data_[pos], at_pos);

				if (at_pos != data.size()) {
					size_t remainder = data.size() - at_pos;
					__detail::copy(data.data() + at_pos, data_, remainder);
				}
			}
			else {
				for (size_t i = 0; i < data.size(); ++i) {
					data[i] = std::move(data_[(read_pos + i) & size_mask]);
					std::destroy_at(&data_[(read_pos + i) & size_mask]);
				}
			}

			read_pos += data.size();
		}

		constexpr T read() {
			T& elem = data_[read_pos++ & size_mask];
			T value(std::move(elem));
			std::destroy_at(&elem);
			return value;
		}

		constexpr void clear() {
			if constexpr (!is_destructible) {
				for (size_t i = 0; i < size(); ++i) {
					std::destroy_at(&data_[(read_pos + i) & size_mask]);
				}
			}

			read_pos = 0;
			write_pos = 0;
		}

		constexpr ~ring_buffer() requires is_destructible = default;
		constexpr ~ring_buffer() {
			clear();
		}

	private:
		T* data_ {};
		size_t size_ {};
		size_t size_mask {};
		size_t read_pos {};
		size_t write_pos {};
	};
}
