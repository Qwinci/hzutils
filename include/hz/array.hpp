#pragma once

#include "config.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#include "utility.hpp"
#else
#include <type_traits>
#include <utility>
#endif

#include <stddef.h>

namespace hz {
	template<typename T, size_t N>
	struct array {
		constexpr T& operator[](size_t index) {
			return __data[index];
		}

		constexpr const T& operator[](size_t index) const {
			return __data[index];
		}

		constexpr T& front() {
			return __data[0];
		}

		constexpr const T& front() const {
			return __data[0];
		}

		constexpr T& back() {
			return __data[N - 1];
		}

		constexpr const T& back() const {
			return __data[N - 1];
		}

		constexpr T* data() {
			return __data;
		}

		constexpr const T* data() const {
			return __data;
		}

		constexpr T* begin() {
			return __data;
		}

		constexpr const T* begin() const {
			return __data;
		}

		constexpr T* end() {
			return __data + N;
		}

		constexpr const T* end() const {
			return __data + N;
		}

		[[nodiscard]] constexpr size_t size() const {
			return N;
		}

		T __data[N];
	};

	namespace __detail {
		template<typename T, size_t N, size_t... I>
		constexpr array<std::remove_cv_t<T>, N> to_array(T (&arr)[N], std::index_sequence<I...>) {
			return {{arr[I]...}};
		}

		template<typename T, size_t N, size_t... I>
		constexpr array<std::remove_cv_t<T>, N> to_array(T (&&arr)[N], std::index_sequence<I...>) {
			return {{std::move(arr[I])...}};
		}
	}

	template<typename T, size_t N>
	constexpr array<std::remove_cv_t<T>, N> to_array(T (&arr)[N]) {
		return __detail::to_array(arr, std::make_index_sequence<N> {});
	}

	template<typename T, size_t N>
	constexpr array<std::remove_cv_t<T>, N> to_array(T (&&arr)[N]) {
		return __detail::to_array(std::move(arr), std::make_index_sequence<N> {});
	}
}
