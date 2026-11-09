#pragma once

#include "config.hpp"
#include "optional.hpp"
#include "allocator.hpp"
#include "stl_allocator.hpp"
#include "hash_impl.hpp"

#include <stddef.h>

#if HZ_FREESTANDING
#include "memory.hpp"
#include "utility.hpp"
#include "initializer_list.hpp"
#include "type_traits.hpp"
#else
#include <memory>
#include <utility>
#include <initializer_list>
#include <type_traits>
#endif

namespace hz {
	template<typename T, allocator<T> Allocator = stl_allocator<T>>
	class vector {
		using alloc_res = alloc_result<Allocator::infallible>;

	public:
		constexpr vector() = default;

		constexpr explicit vector(Allocator alloc) : alloc {std::move(alloc)} {}

		constexpr vector(vector&& other) noexcept
			: data_ {other.data_}, size_ {other.size_}, cap {other.cap}, alloc {std::move(other.alloc)} {

			other.data_ = nullptr;
			other.size_ = 0;
			other.cap = 0;
		}

		vector(const vector&) requires (!Allocator::infallible) = delete;
		constexpr vector(const vector& other) requires Allocator::infallible : alloc {other.alloc} {
			if (other.size_) {
				data_ = alloc.allocate(other.size_);
			}

			size_ = other.size_;
			cap = other.size_;

			for (size_t i = 0; i < size_; ++i) {
				std::construct_at(&data_[i], other.data_[i]);
			}
		}

		constexpr vector(std::initializer_list<T> list) requires Allocator::infallible {
			copy_from(list);
		}

		vector& operator=(const vector&) requires (!Allocator::infallible) = delete;
		constexpr vector& operator=(const vector& other) requires Allocator::infallible {
			copy_from(other);
			return *this;
		}

		constexpr vector& operator=(std::initializer_list<T> list) requires Allocator::infallible {
			copy_from(list);
			return *this;
		}

		constexpr vector& operator=(vector&& other) noexcept {
			if (data_) {
				for (size_t i = 0; i < size_; ++i) {
					std::destroy_at(&data_[i]);
				}

				alloc.deallocate(data_, cap);
			}

			data_ = other.data_;
			size_ = other.size_;
			cap = other.cap;
			alloc = std::move(other.alloc);

			other.data_ = nullptr;
			other.size_ = 0;
			other.cap = 0;

			return *this;
		}

		constexpr T& operator[](size_t index) {
			return data_[index];
		}

		constexpr const T& operator[](size_t index) const {
			return data_[index];
		}

		constexpr alloc_res copy_from(const vector& other) {
			if (other.size_ <= size_) {
				for (size_t i = 0; i < other.size_; ++i) {
					data_[i] = other.data_[i];
				}

				for (size_t i = other.size_; i < size_; ++i) {
					std::destroy_at(&data_[i]);
				}
			}
			else {
				if (auto res = reserve(other.size_); !res) {
					return res;
				}

				for (size_t i = 0; i < size_; ++i) {
					data_[i] = other.data_[i];
				}

				for (size_t i = size_; i < other.size_; ++i) {
					std::construct_at(&data_[i], other.data_[i]);
				}
			}

			size_ = other.size_;

			return alloc_res::success();
		}

		constexpr alloc_res copy_from(std::initializer_list<T> list) {
			if (list.size() <= size_) {
				for (size_t i = 0; i < list.size(); ++i) {
					data_[i] = list.begin()[i];
				}

				for (size_t i = list.size(); i < size_; ++i) {
					std::destroy_at(&data_[i]);
				}
			}
			else {
				if (auto res = reserve(list.size()); !res) {
					return res;
				}

				for (size_t i = 0; i < size_; ++i) {
					data_[i] = list.begin()[i];
				}

				for (size_t i = size_; i < list.size(); ++i) {
					std::construct_at(&data_[i], list.begin()[i]);
				}
			}

			size_ = list.size();

			return alloc_res::success();
		}

		template<typename... Args>
		constexpr alloc_res emplace_back(Args&&... args) {
			if (size_ == cap) {
				size_t new_cap;
				if (cap) {
					new_cap = cap * 2;
				}
				else {
					new_cap = sizeof(T) < 256 ? (256 / sizeof(T)) : 1;
				}

				T* new_data = alloc.allocate(new_cap);
				if (!new_data) {
					return alloc_res::fail();
				}

				std::construct_at(&new_data[size_], std::forward<Args>(args)...);

				for (size_t i = 0; i < size_; ++i) {
					std::construct_at(&new_data[i], std::move(data_[i]));
					std::destroy_at(&data_[i]);
				}

				if (data_) {
					alloc.deallocate(data_, cap);
				}

				data_ = new_data;
				cap = new_cap;
			}
			else {
				std::construct_at(&data_[size_], std::forward<Args>(args)...);
			}

			++size_;

			return alloc_res::success();
		}

		constexpr alloc_res insert(T* pos, const T& value) {
			size_t index = pos - data_;
			T tmp(value);

			if (auto res = reserve(size_ + 1); !res) {
				return res;
			}

			pos = data_ + index;

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

			return alloc_res::success();
		}

		constexpr alloc_res insert(T* pos, T&& value) {
			size_t index = pos - data_;

			if (auto res = reserve(size_ + 1); !res) {
				return res;
			}

			pos = data_ + index;

			if (pos != end()) {
				std::construct_at(end(), std::move(back()));

				for (T* p = end() - 1; p != pos; --p) {
					*p = std::move(*(p - 1));
				}

				*pos = std::move(value);
			}
			else {
				std::construct_at(pos, std::move(value));
			}

			++size_;

			return alloc_res::success();
		}

		constexpr alloc_res push(const T& value) {
			return emplace_back(value);
		}

		constexpr alloc_res push(T&& value) {
			return emplace_back(std::move(value));
		}

		constexpr T pop() {
			T value {std::move(data_[--size_])};
			std::destroy_at(&data_[size_]);
			return value;
		}

		constexpr optional<T> try_pop() {
			if (!size_) {
				return hz::nullopt;
			}

			T value {std::move(data_[--size_])};
			std::destroy_at(&data_[size_]);
			return value;
		}

		constexpr void clear() {
			for (size_t i = 0; i < size_; ++i) {
				std::destroy_at(&data_[i]);
			}

			size_ = 0;
		}

		constexpr T* erase(T* pos) {
			for (size_t i = pos - data_; i < size_ - 1; ++i) {
				data_[i] = std::move(data_[i + 1]);
			}

			std::destroy_at(&data_[--size_]);
			return pos;
		}

		constexpr alloc_res resize(size_t new_size) {
			if (new_size < size_) {
				for (size_t i = new_size; i < size_; ++i) {
					std::destroy_at(&data_[i]);
				}
			}
			else if (new_size > size_) {
				if (auto res = reserve(new_size); !res) {
					return res;
				}

				for (size_t i = size_; i < new_size; ++i) {
					std::construct_at(&data_[i]);
				}
			}

			size_ = new_size;

			return alloc_res::success();
		}

		constexpr alloc_res resize(size_t new_size, const T& value) {
			if (new_size < size_) {
				for (size_t i = new_size; i < size_; ++i) {
					std::destroy_at(&data_[i]);
				}
			}
			else if (new_size > size_) {
				if (new_size > cap) {
					T tmp(value);

					if (auto res = reserve(new_size); !res) {
						return res;
					}

					for (size_t i = size_; i < new_size; ++i) {
						std::construct_at(&data_[i], tmp);
					}
				}
				else {
					for (size_t i = size_; i < new_size; ++i) {
						std::construct_at(&data_[i], value);
					}
				}
			}

			size_ = new_size;

			return alloc_res::success();
		}

		constexpr alloc_res resize_for_overwrite(size_t new_size) {
			if (new_size < size_) {
				for (size_t i = new_size; i < size_; ++i) {
					std::destroy_at(&data_[i]);
				}
			}
			else if (new_size > cap) {
				if (auto res = reserve(new_size); !res) {
					return res;
				}
			}

			size_ = new_size;

			return alloc_res::success();
		}

		constexpr alloc_res reserve(size_t new_cap) {
			if (new_cap <= cap) {
				return alloc_res::success();
			}
			else {
				if (new_cap < cap * 2) {
					new_cap = cap * 2;
				}
				T* new_data = alloc.allocate(new_cap);
				if (!new_data) {
					return alloc_res::fail();
				}

				for (size_t i = 0; i < size_; ++i) {
					std::construct_at(&new_data[i], std::move(data_[i]));
					std::destroy_at(&data_[i]);
				}

				if (data_) {
					alloc.deallocate(data_, cap);
				}

				data_ = new_data;
				cap = new_cap;

				return alloc_res::success();
			}
		}

		constexpr T& front() {
			return data_[0];
		}

		constexpr const T& front() const {
			return data_[0];
		}

		constexpr T& back() {
			return data_[size_ - 1];
		}

		constexpr const T& back() const {
			return data_[size_ - 1];
		}

		constexpr T* data() {
			return data_;
		}

		constexpr const T* data() const {
			return data_;
		}

		constexpr T* begin() {
			return data_;
		}

		constexpr const T* begin() const {
			return data_;
		}

		constexpr T* end() {
			return data_ + size_;
		}

		constexpr const T* end() const {
			return data_ + size_;
		}

		[[nodiscard]] constexpr size_t size() const {
			return size_;
		}

		[[nodiscard]] constexpr size_t capacity() const {
			return cap;
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return !size_;
		}

		constexpr ~vector() {
			for (size_t i = 0; i < size_; ++i) {
				std::destroy_at(&data_[i]);
			}

			if (data_) {
				alloc.deallocate(data_, cap);
			}
		}

	private:
		T* data_ {};
		size_t size_ {};
		size_t cap {};
#if defined(_MSC_VER) && !defined(__clang__)
		[[msvc::no_unique_address]]
#else
		[[no_unique_address]]
#endif
		Allocator alloc {};
	};

	template<typename T, typename Allocator, hasher Hasher>
	struct hash_impl<vector<T, Allocator>, Hasher> {
		static constexpr void hash(const vector<T, Allocator>& self, Hasher& hasher) {
			for (auto& elem : self) {
				hasher.hash(elem);
			}

			hasher.hash(self.size());
		}
	};
}
