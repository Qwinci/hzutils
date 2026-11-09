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
	template<typename F>
	class [[nodiscard]] scope_guard {
	public:
		template<typename Fn> requires std::is_constructible_v<F, Fn>
		constexpr explicit scope_guard(Fn&& f) : fn {std::forward<Fn>(f)} {}

		scope_guard(const scope_guard&) = delete;
		scope_guard& operator=(const scope_guard&) = delete;

		[[nodiscard]] constexpr bool is_armed() const {
			return armed;
		}

		constexpr void disarm() {
			armed = false;
		}

		constexpr ~scope_guard() {
			if (armed) {
				fn();
			}
		}

	private:
		F fn;
		bool armed {true};
	};

	template<typename F>
	scope_guard(F) -> scope_guard<F>;
}
