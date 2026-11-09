#pragma once

#include "config.hpp"
#include "default_delete.hpp"

#if HZ_FREESTANDING
#include "utility.hpp"
#include "cstddef.hpp"
#else
#include <utility>
#include <cstddef>
#endif

namespace hz {
	template<typename T, typename Deleter = default_delete<T>>
	class unique_ptr {
	public:
		friend constexpr void swap(unique_ptr& a, unique_ptr& b) {
			using std::swap;
			swap(a.ptr, b.ptr);
			swap(a.deleter, b.deleter);
		}

		constexpr unique_ptr() = default;
		constexpr unique_ptr(std::nullptr_t) {}

		template<typename U = T, typename D = Deleter>
		constexpr explicit unique_ptr(U* ptr, D&& deleter = {}) : ptr {ptr}, deleter {std::forward<D>(deleter)} {}

		unique_ptr(const unique_ptr& other) = delete;
		unique_ptr& operator=(const unique_ptr& other) = delete;

		template<typename U = T, typename D = Deleter>
		constexpr unique_ptr(unique_ptr<U, D>&& other) noexcept : ptr {other.ptr}, deleter {std::move(other.deleter)} {
			other.ptr = nullptr;
		}

		template<typename U = T, typename D = Deleter>
		constexpr unique_ptr& operator=(unique_ptr<U, D>&& other) noexcept {
			if (ptr) {
				deleter(ptr);
			}
			ptr = other.ptr;
			other.ptr = nullptr;
			deleter = std::move(other.deleter);
			return *this;
		}

		constexpr unique_ptr& operator=(std::nullptr_t) {
			reset();
			return *this;
		}

		template<typename U = T>
		constexpr void reset(U* new_ptr = nullptr) noexcept {
			if (ptr) {
				deleter(ptr);
			}
			ptr = new_ptr;
		}

		constexpr T* release() {
			T* p = ptr;
			ptr = nullptr;
			return p;
		}

		[[nodiscard]] constexpr T* get() {
			return ptr;
		}

		[[nodiscard]] constexpr const T* get() const {
			return ptr;
		}

		constexpr T* operator->() {
			return ptr;
		}

		constexpr const T* operator->() const {
			return ptr;
		}

		constexpr T& operator*() {
			return *ptr;
		}

		constexpr const T& operator*() const {
			return *ptr;
		}

		constexpr explicit operator bool() const {
			return ptr;
		}

		constexpr ~unique_ptr() {
			if (ptr) {
				deleter(ptr);
			}
		}

	private:
		T* ptr {};
#if defined(_MSC_VER) && !defined(__clang__)
		[[msvc::no_unique_address]]
#else
		[[no_unique_address]]
#endif
		Deleter deleter {};
	};

	template<typename Deleter>
	class unique_ptr<void, Deleter> {
	public:
		friend constexpr void swap(unique_ptr& a, unique_ptr& b) {
			using std::swap;
			swap(a.ptr, b.ptr);
			swap(a.deleter, b.deleter);
		}

		constexpr unique_ptr() = default;
		constexpr unique_ptr(std::nullptr_t) {}

		template<typename D = Deleter>
		constexpr explicit unique_ptr(void* ptr, D&& deleter = {}) : ptr {ptr}, deleter {std::forward<D>(deleter)} {}

		unique_ptr(const unique_ptr& other) = delete;
		unique_ptr& operator=(const unique_ptr& other) = delete;

		constexpr unique_ptr(unique_ptr&& other) noexcept : ptr {other.ptr}, deleter {std::move(other.deleter)} {
			other.ptr = nullptr;
		}

		constexpr unique_ptr& operator=(unique_ptr&& other) noexcept {
			if (ptr) {
				deleter(ptr);
			}
			ptr = other.ptr;
			other.ptr = nullptr;
			deleter = std::move(other.deleter);
			return *this;
		}

		constexpr unique_ptr& operator=(std::nullptr_t) {
			reset();
			return *this;
		}

		constexpr void reset(void* new_ptr = nullptr) noexcept {
			if (ptr) {
				deleter(ptr);
			}
			ptr = new_ptr;
		}

		constexpr void* release() {
			void* p = ptr;
			ptr = nullptr;
			return p;
		}

		[[nodiscard]] constexpr void* get() {
			return ptr;
		}

		[[nodiscard]] constexpr const void* get() const {
			return ptr;
		}

		constexpr explicit operator bool() const {
			return ptr;
		}

		constexpr ~unique_ptr() {
			if (ptr) {
				deleter(ptr);
			}
		}

		private:
		void* ptr {};
#if defined(_MSC_VER) && !defined(__clang__)
		[[msvc::no_unique_address]]
#else
		[[no_unique_address]]
#endif
		Deleter deleter {};
	};
}
