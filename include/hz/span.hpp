#pragma once

#include "config.hpp"
#include "array.hpp"
#include "hash_impl.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#include "cstddef.hpp"
#else
#include <type_traits>
#include <cstddef>
#endif

namespace hz {
	template<typename T>
	class span {
	public:
		constexpr span() noexcept = default;

		template<typename It> requires requires(It it) {
			*it;
			++it;
			it != it;
		}
		constexpr span(It first, size_t count) : data_ {to_ptr(first)}, size_ {count} {}
		constexpr span(T* data, size_t count) : data_ {data}, size_ {count} {}

		template<typename It, typename End> requires requires(It it, End end) {
			*it;
			++it;
			it != it;
			end - it;
		}
		constexpr span(It first, End last) : data_ {to_ptr(first)}, size_ {static_cast<size_t>(last - first)} {}

		template<size_t N>
		constexpr span(std::type_identity_t<T> (&arr)[N]) noexcept : data_ {&arr[0]}, size_ {N} {}

		template<typename U> requires std::is_convertible_v<U (*)[], T (*)[]>
		constexpr span(span<U> other) noexcept : data_ {other.data_}, size_ {other.size_} {}

		template<typename C> requires requires(C& c) {
			c.data();
			c.size();
		}
		constexpr span(C& c) : data_ {c.data()}, size_ {c.size()} {}

		[[nodiscard]] constexpr T* begin() const noexcept {
			return data_;
		}

		[[nodiscard]] constexpr T* end() const noexcept {
			return data_ + size_;
		}

		[[nodiscard]] constexpr T& front() const {
			return data_[0];
		}

		[[nodiscard]] constexpr T& back() const {
			return data_[size_ - 1];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) const {
			return data_[index];
		}

		[[nodiscard]] constexpr T* data() const {
			return data_;
		}

		[[nodiscard]] constexpr size_t size() const {
			return size_;
		}

		[[nodiscard]] constexpr size_t size_bytes() const {
			return size_ * sizeof(T);
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return !size_;
		}

		[[nodiscard]] constexpr span first(size_t count) const {
			return {data_, data_ + count};
		}

		[[nodiscard]] constexpr span last(size_t count) const {
			return {data_ + size_ - count, count};
		}

		[[nodiscard]] constexpr span subspan(size_t start, size_t end) const {
			return {data_ + start, data_ + end};
		}

		[[nodiscard]] span<const std::byte> as_bytes() const requires std::is_const_v<T> {
			return {reinterpret_cast<const std::byte*>(data_), size_bytes()};
		}

		[[nodiscard]] span<std::byte> as_bytes() const {
			return {reinterpret_cast<std::byte*>(data_), size_bytes()};
		}

	private:
		template<typename U>
		friend class span;

		template<typename It>
		static constexpr T* to_ptr(It it) {
			if constexpr (requires { static_cast<T*>(it); }) {
				return it;
			}
			else {
				return &*it;
			}
		}

		T* data_ {};
		size_t size_ {};
	};

	template<typename T, size_t N>
	span(T (&)[N]) -> span<T>;

	template<typename T, hasher Hasher>
	struct hash_impl<span<T>, Hasher> {
		static constexpr void hash(const span<T>& self, Hasher& hasher) {
			for (auto& elem : self) {
				hasher.hash(elem);
			}

			hasher.hash(self.size());
		}
	};
}
