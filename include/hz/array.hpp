#pragma once
#include <stddef.h>

namespace hz {
	template<typename T, size_t N>
	struct array {
		constexpr T* data() {
			return __data;
		}

		constexpr const T* data() const {
			return __data;
		}

		constexpr T& operator[](size_t index) {
			return __data[index];
		}

		constexpr const T& operator[](size_t index) const {
			return __data[index];
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

		[[nodiscard]] constexpr size_t size() const {
			return N;
		}

		T __data[N];
	};

	template<typename T, typename...Args>
	array(T, Args...) -> array<T, sizeof...(Args) + 1>;
}
