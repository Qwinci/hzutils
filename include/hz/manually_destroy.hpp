#pragma once
#if __STDC_HOSTED__ == 1
#include <utility>
#include <new>
#else
#include "utility.hpp"
#include "new.hpp"
#endif

namespace hz {
	template<typename T>
	class manually_destroy {
	public:
		template<typename... Args>
		constexpr explicit manually_destroy(Args&&... args) {
			new (storage) T {std::forward<Args&&>(args)...};
		}

		constexpr manually_destroy(const manually_destroy&) = delete;
		constexpr manually_destroy& operator=(const manually_destroy&) = delete;

		constexpr T& operator*() {
			return *std::launder(reinterpret_cast<T*>(storage));
		}

		constexpr const T& operator*() const {
			return *std::launder(reinterpret_cast<T*>(storage));
		}

		constexpr T* operator->() {
			return std::launder(reinterpret_cast<T*>(storage));
		}

		constexpr const T* operator->() const {
			return std::launder(reinterpret_cast<T*>(storage));
		}

		constexpr T* data() {
			return std::launder(reinterpret_cast<T*>(storage));
		}

		constexpr const T* data() const {
			return std::launder(reinterpret_cast<T*>(storage));
		}

		constexpr void destroy() {
			std::launder(reinterpret_cast<T*>(storage))->~T();
		}

	private:
		alignas(T) char storage[sizeof(T)];
	};
}
