#pragma once

#include "config.hpp"
#include "atomic.hpp"
#include "default_delete.hpp"

#if HZ_FREESTANDING
#include "utility.hpp"
#include "cstddef.hpp"
#else
#include <utility>
#include <cstddef>
#endif

namespace hz {
	class refcountable {
	public:
		constexpr refcountable() = default;

		constexpr refcountable(const refcountable&) {}

		constexpr refcountable& operator=(const refcountable&) {
			return *this;
		}

		void acquire_ref() {
			count.fetch_add(1, memory_order::relaxed);
		}

		[[nodiscard]] bool release_ref() {
			return count.fetch_sub(1, memory_order::acq_rel) == 1;
		}

	private:
		atomic<int> count {1};
	};

	template<typename T, typename Deleter = default_delete<T>>
	class ref_ptr {
	public:
		friend constexpr void swap(ref_ptr& a, ref_ptr& b) {
			using std::swap;
			swap(a.ptr, b.ptr);
			swap(a.deleter, b.deleter);
		}

		constexpr ref_ptr() = default;
		constexpr ref_ptr(std::nullptr_t) {}

		template<typename U = T, typename D = Deleter>
		constexpr explicit ref_ptr(U* ptr, D&& deleter = {}) : ptr {ptr}, deleter {std::forward<D>(deleter)} {}

		constexpr ref_ptr(const ref_ptr& other) : ptr {other.ptr}, deleter {other.deleter} {
			if (ptr) {
				ptr->acquire_ref();
			}
		}

		template<typename U, typename D>
		constexpr ref_ptr(const ref_ptr<U, D>& other) : ptr {other.ptr}, deleter {other.deleter} {
			if (ptr) {
				ptr->acquire_ref();
			}
		}

		constexpr ref_ptr(ref_ptr&& other) noexcept : ptr {other.ptr}, deleter {std::move(other.deleter)} {
			other.ptr = nullptr;
		}

		template<typename U, typename D>
		constexpr ref_ptr(ref_ptr<U, D>&& other) noexcept : ptr {other.ptr}, deleter {std::move(other.deleter)} {
			other.ptr = nullptr;
		}

		constexpr ref_ptr& operator=(ref_ptr other) noexcept {
			swap(*this, other);
			return *this;
		}

		constexpr ref_ptr& operator=(std::nullptr_t) {
			reset();
			return *this;
		}

		template<typename U = T>
		constexpr void reset(U* new_ptr = nullptr) noexcept {
			if (ptr) {
				if (ptr->release_ref()) {
					deleter(ptr);
				}
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

		constexpr ~ref_ptr() {
			reset();
		}

	private:
		template<typename, typename>
		friend class ref_ptr;

		T* ptr {};
#if defined(_MSC_VER) && !defined(__clang__)
		[[msvc::no_unique_address]]
#else
		[[no_unique_address]]
#endif
		Deleter deleter {};
	};

	template<typename T>
	ref_ptr(T*) -> ref_ptr<T>;
}
