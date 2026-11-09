#pragma once

#include "config.hpp"

#if HZ_FREESTANDING
#include "memory.hpp"
#include "new.hpp"
#else
#include <memory>
#include <new>
#endif

namespace hz {
	template<typename T>
	struct stl_allocator {
		static constexpr bool infallible = true;

		static constexpr T* allocate(size_t count) {
			return std::allocator<T> {}.allocate(count);
		}

		static constexpr void deallocate(T* ptr, size_t count) {
			if (ptr) {
				std::allocator<T> {}.deallocate(ptr, count);
			}
		}

		static unsigned char* allocate_bytes(size_t size) {
			if constexpr (alignof(T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
				return static_cast<unsigned char*>(::operator new(size));
			}
			else {
				return static_cast<unsigned char*>(::operator new(size, static_cast<std::align_val_t>(alignof(T))));
			}
		}

		static void deallocate_bytes(unsigned char* ptr, size_t size) {
			if constexpr (alignof(T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
				::operator delete(ptr, size);
			}
			else {
				::operator delete(ptr, size, static_cast<std::align_val_t>(alignof(T)));
			}
		}
	};
}
