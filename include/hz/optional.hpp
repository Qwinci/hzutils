#pragma once
#if __STDC_HOSTED__ == 1
#include <new>
#include <utility>
#else
#include "new.hpp"
#include "utility.hpp"
#endif

namespace hz {
	struct nullopt_t {};

	inline constexpr nullopt_t nullopt {};

	template<typename T>
	class optional {
	public:
		constexpr optional() : data {}, _has_value {false} {}

		constexpr optional(nullopt_t) : data {}, _has_value {false} {} // NOLINT(*-explicit-constructor)

		constexpr optional(optional&& other) noexcept : data {}, _has_value {} {
			if (other._has_value) {
				new (&data) T {std::move(*std::launder(reinterpret_cast<T*>(other.data)))};
				_has_value = true;
			}
		}

		constexpr optional(const optional& other) : data {}, _has_value {} {
			if (other._has_value) {
				new (&data) T {*std::launder(reinterpret_cast<const T*>(other.data))};
				_has_value = true;
			}
		}

		template<typename U = T> requires(!is_same_v<remove_cvref_t<U>, optional>)
		constexpr optional(U&& value) : data {}, _has_value {true} { // NOLINT(*-explicit-constructor)
			new (&data) T {std::forward<U&&>(value)};
		}

		constexpr optional& operator=(nullopt_t) {
			if (_has_value) {
				std::launder(reinterpret_cast<T*>(data))->~T();
				_has_value = false;
			}
			return *this;
		}

		constexpr optional& operator=(const optional& other) {
			if (&other == this) {
				return *this;
			}

			if (other._has_value) {
				auto other_value = std::launder(reinterpret_cast<const T*>(other.data));
				if (_has_value) {
					*std::launder(reinterpret_cast<T*>(data)) = *other_value;
				}
				else {
					new (&data) T {*other_value};
					_has_value = true;
				}
			}
			else {
				if (_has_value) {
					std::launder(reinterpret_cast<T*>(data))->~T();
					_has_value = false;
				}
			}

			return *this;
		}

		constexpr optional& operator=(optional&& other) noexcept {
			if (other._has_value) {
				auto other_value = std::launder(reinterpret_cast<T*>(other.data));
				if (_has_value) {
					*std::launder(reinterpret_cast<T*>(data)) = std::move(*other_value);
				}
				else {
					new (&data) T {std::move(*other_value)};
					_has_value = true;
				}
			}
			else {
				if (_has_value) {
					std::launder(reinterpret_cast<T*>(data))->~T();
					_has_value = false;
				}
			}

			return *this;
		}

		template<typename U = T> requires(!is_same_v<remove_cvref_t<U>, optional>)
		constexpr optional& operator=(U&& value) noexcept {
			if (_has_value) {
				*std::launder(reinterpret_cast<T*>(data)) = std::forward<U>(value);
			}
			else {
				new (&data) T {std::forward<U>(value)};
				_has_value = true;
			}

			return *this;
		}

		constexpr ~optional() {
			if (_has_value) {
				std::launder(reinterpret_cast<T*>(data))->~T();
			}
		}

		constexpr T* operator->() noexcept {
			return std::launder(reinterpret_cast<T*>(data));
		}

		constexpr const T* operator->() const noexcept {
			return std::launder(reinterpret_cast<T*>(data));
		}

		constexpr T& operator*() & noexcept {
			return *std::launder(reinterpret_cast<T*>(data));
		}

		constexpr const T& operator*() const & noexcept {
			return *std::launder(reinterpret_cast<T*>(data));
		}

		constexpr T&& operator*() && noexcept {
			return std::move(*std::launder(reinterpret_cast<T*>(data)));
		}

		constexpr const T&& operator*() const && noexcept {
			return std::move(*std::launder(reinterpret_cast<T*>(data)));
		}

		constexpr explicit operator bool() const noexcept {
			return _has_value;
		}

		[[nodiscard]] constexpr bool has_value() const noexcept {
			return _has_value;
		}

		constexpr T& value() & {
			return *std::launder(reinterpret_cast<T*>(data));
		}

		constexpr const T& value() const & {
			return *std::launder(reinterpret_cast<T*>(data));
		}

		constexpr T&& value() && {
			return std::move(*std::launder(reinterpret_cast<T*>(data)));
		}

		constexpr const T&& value() const && {
			return std::move(*std::launder(reinterpret_cast<T*>(data)));
		}

		constexpr void reset() noexcept {
			if (_has_value) {
				std::launder(reinterpret_cast<T*>(data))->~T();
				_has_value = false;
			}
		}

		template<typename... Args>
		constexpr T& emplace(Args&&... args) {
			if (_has_value) {
				std::launder(reinterpret_cast<T*>(data))->~T();
			}
			return *new (&data) T {std::forward<Args>(args)...};
		}

	private:
		alignas(T) char data[sizeof(T)];
		bool _has_value;
	};

	template<typename T>
	optional(T) -> optional<T>;
}
