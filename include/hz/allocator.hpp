#pragma once

#include "config.hpp"

#include <stddef.h>

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

#if HZ_FREESTANDING
#include "concepts.hpp"
#include "utility.hpp"
#else
#include <concepts>
#include <utility>
#endif

namespace hz {
	template<typename Self, typename T>
	concept allocator = requires(Self& self, size_t count, T* ptr, unsigned char* bytes) {
		static_cast<bool>(Self::infallible);
		{ self.allocate(count) } -> std::same_as<T*>;
		{ self.deallocate(ptr, count) } -> std::same_as<void>;
		{ self.allocate_bytes(count) } -> std::same_as<unsigned char*>;
		{ self.deallocate_bytes(bytes, count) } -> std::same_as<void>;
	};

	template<bool Infallible, typename T = void>
	struct alloc_result;

	template<typename T>
	struct alloc_result<true, T> {
		static constexpr alloc_result success(T value) {
			return {std::move(value)};
		}

		[[noreturn]] static alloc_result fail() {
#if defined(_MSC_VER) && !defined(__clang__)
			__fastfail(7);
			__assume(false);
#else
			__builtin_trap();
#endif
		}

		constexpr explicit operator bool() const {
			return true;
		}

		T value;
	};

	template<>
	struct alloc_result<true, void> {
		static constexpr alloc_result success() {
			return {};
		}

		[[noreturn]] static alloc_result fail() {
#if defined(_MSC_VER) && !defined(__clang__)
			__fastfail(7);
			__assume(false);
#else
			__builtin_trap();
#endif
		}

		constexpr explicit operator bool() const {
			return true;
		}
	};

	template<>
	struct [[nodiscard]] alloc_result<false, void> {
		bool __success;

		static constexpr alloc_result success() {
			return {true};
		}

		static constexpr alloc_result fail() {
			return {false};
		}

		constexpr explicit operator bool() const {
			return __success;
		}
	};

	template<typename T>
	struct [[nodiscard]] alloc_result<false, T> {
		T value;
		bool __success;

		static constexpr alloc_result success(T value) {
			return {std::move(value), true};
		}

		static constexpr alloc_result fail() {
			return {{}, false};
		}

		constexpr explicit operator bool() const {
			return __success;
		}
	};
}
