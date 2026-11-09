#pragma once

#include <stdint.h>

#if HZ_FREESTANDING
#include "initializer_list.hpp"
#include "type_traits.hpp"
#else
#include <initializer_list>
#include <type_traits>
#endif

namespace hz {
	template<typename T>
	constexpr const T& min(const T& a, const T& b) {
		return b < a ? b : a;
	}

	template<typename T>
	constexpr T min(std::initializer_list<T> list) {
		auto it = list.begin();
		T value = *it++;

		for (; it != list.end(); ++it) {
			if (*it < value) {
				value = *it;
			}
		}

		return value;
	}

	template<typename T>
	constexpr const T& max(const T& a, const T& b) {
		return a < b ? b : a;
	}

	template<typename T>
	constexpr T max(std::initializer_list<T> list) {
		auto it = list.begin();
		T value = *it++;

		for (; it != list.end(); ++it) {
			if (value < *it) {
				value = *it;
			}
		}

		return value;
	}

	template<typename T>
	constexpr const T& clamp(const T& value, const T& low, const T& high) {
		return value < low ? low : (value > high ? high : value);
	}

	template<typename T>
	constexpr std::make_unsigned_t<T> abs(T value) {
		if constexpr (std::is_signed_v<T>) {
			if (value < 0) {
				return static_cast<std::make_unsigned_t<T>>(-static_cast<std::make_unsigned_t<T>>(value));
			}
			else {
				return static_cast<std::make_unsigned_t<T>>(value);
			}
		}
		else {
			return value;
		}
	}

	template<typename T>
	constexpr T ceil_div(T value, auto divisor) {
		return (value + (static_cast<T>(divisor - 1))) / divisor;
	}

	template<typename T>
	constexpr T pow(T value, auto power) {
		T res = value;

		for (; power > 1; --power) {
			res *= value;
		}

		return res;
	}

	template<typename T>
	constexpr T align_up(T value, auto align) {
		return static_cast<T>((value + (align - 1)) & ~static_cast<T>(align - 1));
	}

	template<typename T>
	inline T* align_up(T* value, auto align) {
		return reinterpret_cast<T*>(align_up(reinterpret_cast<uintptr_t>(value), align));
	}

	template<typename T>
	constexpr T align_down(T value, auto align) {
		return static_cast<T>(value & ~static_cast<T>(align - 1));
	}

	template<typename T>
	inline T* align_down(T* value, auto align) {
		return reinterpret_cast<T*>(align_down(reinterpret_cast<uintptr_t>(value), align));
	}

	template<typename T>
	constexpr bool is_aligned(T value, auto align) {
		return (value & (align - 1)) == 0;
	}

	template<typename T>
	inline bool is_aligned(T* value, auto align) {
		return is_aligned(reinterpret_cast<uintptr_t>(value), align);
	}

	template<typename T>
	constexpr T misalign(T value, auto align) {
		return static_cast<T>(value & (align - 1));
	}

	template<typename T>
	inline uintptr_t misalign(T* value, auto align) {
		return misalign(reinterpret_cast<uintptr_t>(value), align);
	}
}
