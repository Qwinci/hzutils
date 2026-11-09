#pragma once

#include "new.hpp"
#include "utility.hpp"

namespace std {
	template<typename __T, typename... __Args>
	HZ_FORCEINLINE constexpr __T* construct_at(__T* __loc, __Args&&... __args) {
#if defined(_MSC_VER) && !defined(__clang__)
		[[msvc::constexpr]]
#endif
		return ::new (__loc) __T(forward<__Args>(__args)...);
	}

	template<typename __T>
	HZ_FORCEINLINE constexpr void destroy_at(__T* __loc) {
		__loc->~__T();
	}

	template<typename __T>
	struct allocator {
		constexpr allocator() noexcept = default;

		template<typename __U>
		constexpr allocator(const allocator<__U>&) {}

		constexpr __T* allocate(size_t __count) {
			if constexpr (alignof(__T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
#if defined(_MSC_VER) && !defined(__clang__)
				[[msvc::constexpr]]
#endif
				return static_cast<__T*>(::operator new(__count * sizeof(__T)));
			}
			else {
#if defined(_MSC_VER) && !defined(__clang__)
				[[msvc::constexpr]]
#endif
				return static_cast<__T*>(::operator new(__count * sizeof(__T), static_cast<align_val_t>(alignof(__T))));
			}
		}

		constexpr void deallocate(__T* __ptr, size_t __count) {
			if constexpr (alignof(__T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
#if defined(_MSC_VER) && !defined(__clang__)
				[[msvc::constexpr]]
#endif
				::operator delete(__ptr, __count * sizeof(__T));
			}
			else {
#if defined(_MSC_VER) && !defined(__clang__)
				[[msvc::constexpr]]
#endif
				::operator delete(__ptr, __count * sizeof(__T), static_cast<align_val_t>(alignof(__T)));
			}
		}
	};
}
