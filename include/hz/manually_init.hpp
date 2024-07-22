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
	class manually_init {
	public:
		constexpr manually_init() = default;
		constexpr manually_init(const manually_init&) = delete;
		constexpr manually_init& operator=(const manually_init&) = delete;

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

		template<typename... Args>
		constexpr void initialize(Args&&... args) {
			new (storage) T {std::forward<Args&&>(args)...};
		}

		constexpr void destroy() {
			std::launder(reinterpret_cast<T*>(storage))->~T();
		}

	private:
		alignas(T) char storage[sizeof(T)];
	};
}
