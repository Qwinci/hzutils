#pragma once

#include <stddef.h>

constexpr void* operator new(size_t, void* __ptr) noexcept {
	return __ptr;
}

constexpr void* operator new[](size_t, void* __ptr) noexcept {
	return __ptr;
}

namespace std {
	template<typename __T>
	[[nodiscard]] constexpr __T* launder(__T* __ptr) noexcept {
		return __builtin_launder(__ptr);
	}

	struct nothrow_t {
		explicit nothrow_t() = default;
	};

	extern const nothrow_t nothrow;

	enum class align_val_t : size_t {};
}
