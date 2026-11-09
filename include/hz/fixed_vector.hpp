#pragma once

#include "config.hpp"
#include "optional.hpp"
#include "hash_impl.hpp"

#include <stddef.h>

#if HZ_FREESTANDING
#include "memory.hpp"
#include "initializer_list.hpp"
#include "type_traits.hpp"
#include "utility.hpp"
#else
#include <memory>
#include <initializer_list>
#include <type_traits>
#include <utility>
#endif

namespace hz {
	template<typename T, size_t N>
	class fixed_vector {
		static constexpr bool is_copyable = std::is_trivially_copyable_v<T>;

	public:
		constexpr fixed_vector() = default;

		constexpr explicit fixed_vector(size_t count) : size_ {count} {
			for (size_t i = 0; i < count; ++i) {
				std::construct_at(&storage.data[i]);
			}
		}

		constexpr explicit fixed_vector(size_t count, const T& value) : size_ {count} {
			for (size_t i = 0; i < count; ++i) {
				std::construct_at(&storage.data[i], value);
			}
		}

		template<typename It> requires requires(It it) {
			*it;
			++it;
			it != it;
		}
		constexpr fixed_vector(It begin, It end) {
			for (; begin != end; ++begin) {
				std::construct_at(&storage.data[size_++], *begin);
			}
		}

		constexpr fixed_vector(std::initializer_list<T> list) : size_ {list.size()} {
			for (size_t i = 0; i < size_; ++i) {
				std::construct_at(&storage.data[i], list.begin()[i]);
			}
		}

		constexpr fixed_vector(const fixed_vector& other) requires is_copyable = default;
		constexpr fixed_vector(const fixed_vector& other) : size_ {other.size_} {
			for (size_t i = 0; i < size_; ++i) {
				std::construct_at(&storage.data[i], other.storage.data[i]);
			}
		}

		constexpr fixed_vector(fixed_vector&& other) noexcept requires is_copyable = default;
		constexpr fixed_vector(fixed_vector&& other) noexcept : size_ {other.size_} {
			for (size_t i = 0; i < size_; ++i) {
				std::construct_at(&storage.data[i], std::move(other.storage.data[i]));
			}
		}

		constexpr fixed_vector& operator=(const fixed_vector& other) requires is_copyable = default;
		constexpr fixed_vector& operator=(const fixed_vector& other) {
			for (size_t i = 0; i < size_; ++i) {
				std::destroy_at(&storage.data[i]);
			}

			size_ = other.size_;

			for (size_t i = 0; i < size_; ++i) {
				std::construct_at(&storage.data[i], other.storage.data[i]);
			}

			return *this;
		}

		constexpr fixed_vector& operator=(fixed_vector&& other) noexcept requires is_copyable = default;
		constexpr fixed_vector& operator=(fixed_vector&& other) noexcept {
			for (size_t i = 0; i < size_; ++i) {
				std::destroy_at(&storage.data[i]);
			}

			size_ = other.size_;

			for (size_t i = 0; i < size_; ++i) {
				std::construct_at(&storage.data[i], std::move(other.storage.data[i]));
			}

			return *this;
		}

		constexpr fixed_vector& operator=(std::initializer_list<T> list) {
			for (size_t i = 0; i < size_; ++i) {
				std::destroy_at(&storage.data[i]);
			}

			size_ = list.size();

			for (size_t i = 0; i < size_; ++i) {
				std::construct_at(&storage.data[i], list.begin()[i]);
			}

			return *this;
		}

		constexpr T& operator[](size_t index) {
			return storage.data[index];
		}

		constexpr const T& operator[](size_t index) const {
			return storage.data[index];
		}

		constexpr void resize(size_t count) {
			if (count < size_) {
				for (size_t i = count; i < size_; ++i) {
					std::destroy_at(&storage.data[i]);
				}
			}
			else if (count > size_) {
				for (size_t i = size_; i < count; ++i) {
					std::construct_at(&storage.data[i]);
				}
			}

			size_ = count;
		}

		constexpr void resize(size_t count, const T& value) {
			if (count < size_) {
				for (size_t i = count; i < size_; ++i) {
					std::destroy_at(&storage.data[i]);
				}
			}
			else if (count > size_) {
				for (size_t i = size_; i < count; ++i) {
					std::construct_at(&storage.data[i], value);
				}
			}

			size_ = count;
		}

		constexpr void resize_for_overwrite(size_t count) {
			if (count < size_) {
				for (size_t i = count; i < size_; ++i) {
					std::destroy_at(&storage.data[i]);
				}
			}

			size_ = count;
		}

		constexpr T* insert(T* pos, const T& value) {
			T tmp(value);

			if (pos != end()) {
				std::construct_at(end(), std::move(back()));

				for (T* p = end() - 1; p != pos; --p) {
					*p = std::move(*(p - 1));
				}

				*pos = std::move(tmp);
			}
			else {
				std::construct_at(pos, std::move(tmp));
			}

			++size_;
			return pos;
		}

		constexpr T* insert(T* pos, T&& value) {
			if (pos != end()) {
				std::construct_at(end(), std::move(back()));

				for (T* p = end() - 1; p != pos; --p) {
					*p = std::move(*(p - 1));
				}

				*pos = value;
			}
			else {
				std::construct_at(pos, std::move(value));
			}

			++size_;
			return pos;
		}

		template<typename... Args>
		constexpr void emplace_back(Args&&... args) {
			std::construct_at(&storage.data[size_++], std::forward<Args>(args)...);
		}

		template<typename... Args>
		[[nodiscard]] constexpr bool try_emplace_back(Args&&... args) {
			if (size_ == N) {
				return false;
			}

			std::construct_at(&storage.data[size_++], std::forward<Args>(args)...);
			return true;
		}

		constexpr void push(const T& value) {
			std::construct_at(&storage.data[size_++], value);
		}

		[[nodiscard]] constexpr bool try_push(const T& value) {
			if (size_ == N) {
				return false;
			}

			std::construct_at(&storage.data[size_++], value);
			return true;
		}

		constexpr void push(T&& value) {
			std::construct_at(&storage.data[size_++], std::move(value));
		}

		[[nodiscard]] constexpr bool try_push(T&& value) {
			if (size_ == N) {
				return false;
			}

			std::construct_at(&storage.data[size_++], std::move(value));
			return true;
		}

		constexpr T pop() {
			T value {std::move(storage.data[--size_])};
			std::destroy_at(&storage.data[size_]);
			return value;
		}

		constexpr optional<T> try_pop() {
			if (!size_) {
				return hz::nullopt;
			}

			T value {std::move(storage.data[--size_])};
			std::destroy_at(&storage.data[size_]);
			return value;
		}

		constexpr void clear() {
			for (size_t i = 0; i < size_; ++i) {
				std::destroy_at(&storage.data[i]);
			}

			size_ = 0;
		}

		constexpr T* erase(T* pos) {
			for (size_t i = pos - storage.data; i < size_ - 1; ++i) {
				storage.data[i] = std::move(storage.data[i + 1]);
			}

			std::destroy_at(&storage.data[--size_]);
			return pos;
		}

		constexpr T& front() {
			return storage.data[0];
		}

		constexpr const T& front() const {
			return storage.data[0];
		}

		constexpr T& back() {
			return storage.data[size_ - 1];
		}

		constexpr const T& back() const {
			return storage.data[size_ - 1];
		}

		constexpr T* data() {
			return storage.data;
		}

		constexpr const T* data() const {
			return storage.data;
		}

		constexpr T* begin() {
			return storage.data;
		}

		constexpr const T* begin() const {
			return storage.data;
		}

		constexpr T* end() {
			return storage.data + size_;
		}

		constexpr const T* end() const {
			return storage.data + size_;
		}

		[[nodiscard]] constexpr size_t size() const {
			return size_;
		}

		static constexpr size_t capacity() noexcept {
			return N;
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return !size_;
		}

		constexpr ~fixed_vector() requires std::is_trivially_destructible_v<T> = default;
		constexpr ~fixed_vector() {
			for (size_t i = 0; i < size_; ++i) {
				std::destroy_at(&storage.data[i]);
			}
		}

	private:
		union storage {
			constexpr storage() {}

			constexpr ~storage() requires std::is_trivially_destructible_v<T> = default;
			constexpr ~storage() {}

			T data[N];
		} storage;

		size_t size_ {};
	};

	template<typename T, size_t N, hasher Hasher>
	struct hash_impl<fixed_vector<T, N>, Hasher> {
		static constexpr void hash(const fixed_vector<T, N>& self, Hasher& hasher) {
			for (auto& elem : self) {
				hasher.hash(elem);
			}

			hasher.hash(self.size());
		}
	};
}
