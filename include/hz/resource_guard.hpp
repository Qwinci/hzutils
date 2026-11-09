#pragma once

#include "config.hpp"

#if HZ_FREESTANDING
#include "utility.hpp"
#include "type_traits.hpp"
#else
#include <utility>
#include <type_traits>
#endif

namespace hz {
	template<typename T, typename F>
	class [[nodiscard]] resource_guard {
	public:
		template<typename Fn> requires std::is_constructible_v<F, Fn>
		constexpr resource_guard(Fn&& release, T* value) : release {std::forward<Fn>(release)}, value {value} {}

		resource_guard(const resource_guard&) = delete;
		resource_guard& operator=(const resource_guard&) = delete;

		T* operator->() {
			return value;
		}

		T& operator*() {
			return *value;
		}

		T& get() {
			return *value;
		}

		constexpr ~resource_guard() {
			release();
		}

	private:
		F release;
		T* value;
	};

	template<typename F>
	class [[nodiscard]] resource_guard<void, F> {
	public:
		template<typename Fn> requires std::is_constructible_v<F, Fn>
		constexpr explicit resource_guard(Fn&& release) : release {std::forward<Fn>(release)} {}

		resource_guard(const resource_guard&) = delete;
		resource_guard& operator=(const resource_guard&) = delete;

		constexpr ~resource_guard() {
			release();
		}

	private:
		F release;
	};

	template<typename F, typename T>
	resource_guard(F, T*) -> resource_guard<T, F>;
	template<typename F>
	resource_guard(F) -> resource_guard<void, F>;
}
