#pragma once
#include "array.hpp"

namespace hz {

	template<typename T>
	struct span {
		constexpr span() = default;

		template<typename U, size_t N>
		constexpr span(array<U, N>& arr) : _begin {arr.begin()}, _end {arr.end()} {} // NOLINT(*-explicit-constructor)

		template<typename U, size_t N>
		constexpr span(const array<U, N>& arr) : _begin {arr.begin()}, _end {arr.end()} {} // NOLINT(*-explicit-constructor)

		template<size_t N>
		constexpr span(T (&arr)[N]) : _begin {&arr[0]}, _end {&arr[N]} {} // NOLINT(*-explicit-constructor)

		constexpr T* begin() {
			return _begin;
		}

		constexpr const T* begin() const {
			return _begin;
		}

		constexpr T* end() {
			return _end;
		}

		constexpr const T* end() const {
			return _end;
		}

		constexpr T* data() {
			return _begin;
		}

		constexpr const T* data() const {
			return _begin;
		}

		[[nodiscard]] constexpr size_t size() const {
			return _end - _begin;
		}

		constexpr T& operator[](size_t index) {
			return _begin[index];
		}

		constexpr const T& operator[](size_t index) const {
			return _begin[index];
		}

	private:
		T* _begin {};
		T* _end {};
	};
}
