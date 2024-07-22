#pragma once
#include "algorithm.hpp"
#include "type_traits.hpp"
#if __STDC_HOSTED__ == 1
#include <new>
#include <utility>
#else
#include "new.hpp"
#include "utility.hpp"
#endif

namespace hz {
	namespace __detail {
		template<typename T>
		struct success_type {
			T value;
		};

		template<typename T>
		struct error_type {
			T value;
		};
	}

	template<typename T>
	constexpr __detail::success_type<T> success(T&& value) {
		return {std::forward<T>(value)};
	}

	template<typename T>
	constexpr __detail::success_type<T> success(const T& value) {
		return {value};
	}

	template<typename T>
	constexpr __detail::error_type<T> error(T&& value) {
		return {std::forward<T>(value)};
	}

	template<typename T>
	constexpr __detail::error_type<T> error(const T& value) {
		return {value};
	}

	template<typename T, typename E>
	class result {
	public:
		constexpr result() : data {}, _success {false} {
			new (&data) E {};
		}

		constexpr result(__detail::success_type<T>&& value) : data {}, _success {true} { // NOLINT(*-explicit-constructor)
			new (&data) T {std::move(value.value)};
		}

		constexpr result(__detail::error_type<E>&& value) : data {}, _success {false} { // NOLINT(*-explicit-constructor)
			new (&data) E {std::move(value.value)};
		}

		constexpr result(result&& other) noexcept : data {}, _success {false} {
			if (other._success) {
				new (&data) T {std::move(*std::launder(reinterpret_cast<T*>(other.data)))};
				_success = true;
			}
			else {
				new (&data) E {std::move(*std::launder(reinterpret_cast<E*>(other.data)))};
			}
		}

		constexpr result(const result& other) : data {}, _success {false} {
			if (other._success) {
				new (&data) T {*std::launder(reinterpret_cast<const T*>(other.data))};
				_success = true;
			}
			else {
				new (&data) E {*std::launder(reinterpret_cast<const E*>(other.data))};
			}
		}

		constexpr result& operator=(const result& other) {
			if (&other == this) {
				return *this;
			}

			if (other._success) {
				auto other_value = std::launder(reinterpret_cast<const T*>(other.data));
				if (_success) {
					*std::launder(reinterpret_cast<T*>(data)) = *other_value;
				}
				else {
					std::launder(reinterpret_cast<E*>(data))->~E();
					new (&data) T {*other_value};
					_success = true;
				}
			}
			else {
				auto other_value = std::launder(reinterpret_cast<const E*>(other.data));
				if (!_success) {
					*std::launder(reinterpret_cast<E*>(data)) = *other_value;
				}
				else {
					std::launder(reinterpret_cast<T*>(data))->~T();
					new (&data) E {*other_value};
					_success = false;
				}
			}

			return *this;
		}

		constexpr result& operator=(result&& other) noexcept {
			if (other._success) {
				auto other_value = std::launder(reinterpret_cast<T*>(other.data));
				if (_success) {
					*std::launder(reinterpret_cast<T*>(data)) = std::move(*other_value);
				}
				else {
					std::launder(reinterpret_cast<E*>(data))->~E();
					new (&data) T {std::move(*other_value)};
					_success = true;
				}
			}
			else {
				auto other_value = std::launder(reinterpret_cast<E*>(other.data));
				if (!_success) {
					*std::launder(reinterpret_cast<E*>(data)) = std::move(*other_value);
				}
				else {
					std::launder(reinterpret_cast<T*>(data))->~T();
					new (&data) E {std::move(*other_value)};
					_success = false;
				}
			}

			return *this;
		}

		constexpr result& operator=(__detail::success_type<T>&& value) noexcept {
			if (_success) {
				*std::launder(reinterpret_cast<T*>(data)) = std::move(value.value);
			}
			else {
				std::launder(reinterpret_cast<E*>(data))->~E();
				new (&data) T {std::move(value.value)};
				_success = true;
			}

			return *this;
		}

		constexpr result& operator=(__detail::error_type<E>&& value) noexcept {
			if (_success) {
				std::launder(reinterpret_cast<T*>(data))->~T();
				new (&data) E {std::move(value.value)};
				_success = false;
			}
			else {
				*std::launder(reinterpret_cast<E*>(data)) = std::move(value.value);
			}

			return *this;
		}

		constexpr ~result() {
			if (_success) {
				std::launder(reinterpret_cast<T*>(data))->~T();
			}
			else {
				std::launder(reinterpret_cast<E*>(data))->~E();
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
			return _success;
		}

		[[nodiscard]] constexpr bool has_value() const noexcept {
			return _success;
		}

		[[nodiscard]] constexpr bool has_error() const noexcept {
			return !_success;
		}

		constexpr E& error() & {
			return *std::launder(reinterpret_cast<E*>(data));
		}

		constexpr const E& error() const & {
			return *std::launder(reinterpret_cast<E*>(data));
		}

		constexpr E&& error() && {
			return std::move(*std::launder(reinterpret_cast<E*>(data)));
		}

		constexpr const E&& error() const && {
			return std::move(*std::launder(reinterpret_cast<E*>(data)));
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

	private:
		alignas(hz::max(alignof(T), alignof(E))) char data[hz::max(sizeof(T), sizeof(E))];
		bool _success;
	};
}
