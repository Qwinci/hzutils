#pragma once

#include "config.hpp"

#if HZ_FREESTANDING
#include "new.hpp"
#include "utility.hpp"
#else
#include <new>
#include <utility>
#endif

namespace hz {
	template<typename T>
	class manually_init {
		T* ptr() {
			return std::launder(reinterpret_cast<T*>(storage));
		}

		const T* ptr() const {
			return std::launder(reinterpret_cast<const T*>(storage));
		}

	public:
		constexpr manually_init() = default;

		manually_init(const manually_init&) = delete;
		manually_init& operator=(const manually_init&) = delete;

		template<typename... Args>
		void initialize(Args&&... args) {
			if (has_value_) {
				ptr()->~T();
			}

			new (storage) T(std::forward<Args>(args)...);
			has_value_ = true;
		}

		void destroy() {
			if (has_value_) {
				ptr()->~T();
				has_value_ = false;
			}
		}

		T& value() {
			return *ptr();
		}

		const T& value() const {
			return *ptr();
		}

		T* operator->() {
			return ptr();
		}

		const T* operator->() const {
			return ptr();
		}

		T& operator*() {
			return *ptr();
		}

		const T& operator*() const {
			return *ptr();
		}

		constexpr bool has_value() const {
			return has_value_;
		}

	private:
		alignas(T) char storage[sizeof(T)] {};
		bool has_value_ {};
	};
}
