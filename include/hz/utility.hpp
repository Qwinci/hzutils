#pragma once
#include "type_traits.hpp"

namespace std {
	template<typename __T>
	constexpr __T&& forward(hz::remove_reference_t<__T>& __value) {
		return static_cast<__T&&>(__value);
	}

	template<typename __T>
	constexpr __T&& forward(hz::remove_reference_t<__T>&& __value) {
		return static_cast<__T&&>(__value);
	}

	template<typename __T>
	constexpr hz::remove_reference_t<__T>&& move(__T&& __value) {
		return static_cast<hz::remove_reference_t<__T>&&>(__value);
	}
}
