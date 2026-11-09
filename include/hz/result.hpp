#pragma once

#include "config.hpp"

#if HZ_FREESTANDING
#include "memory.hpp"
#include "utility.hpp"
#include "type_traits.hpp"
#else
#include <memory>
#include <utility>
#include <type_traits>
#endif

namespace hz {
	namespace __detail { // NOLINT(*-reserved-identifier)
		template<typename T>
		struct success_type {
			T&& value;
		};

		template<typename T>
		struct error_type {
			T&& value;
		};
	}

	template<typename T>
	[[nodiscard]] constexpr __detail::success_type<T> success(T&& value) {
		return __detail::success_type<T> {std::forward<T>(value)};
	}

	template<typename T>
	[[nodiscard]] constexpr __detail::error_type<T> error(T&& value) {
		return __detail::error_type<T> {std::forward<T>(value)};
	}

	template<typename T, typename E>
	class result {
		static constexpr bool is_copyable = std::is_trivially_copyable_v<T> && std::is_trivially_copyable_v<E>;
		static constexpr bool is_destructible = std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>;

	public:
		friend constexpr void swap(result& a, result& b) {
			using std::swap;

			if (a.success_ && b.success_) {
				swap(a.storage.value, b.storage.value);
			}
			else if (!a.success_ && !b.success_) {
				swap(a.storage.error, b.storage.error);
			}
			else if (a.success_) {
				T tmp(std::move(a.storage.value));
				std::destroy_at(&a.storage.value);

				std::construct_at(&a.storage.error, std::move(b.storage.error));
				std::destroy_at(&b.storage.error);

				std::construct_at(&b.storage.value, std::move(tmp));
				swap(a.success_, b.success_);
			}
			else {
				T tmp(std::move(b.storage.value));
				std::destroy_at(&b.storage.value);

				std::construct_at(&b.storage.error, std::move(a.storage.error));
				std::destroy_at(&a.storage.error);

				std::construct_at(&a.storage.value, std::move(tmp));
				swap(a.success_, b.success_);
			}
		}

		constexpr result() : storage {error_tag {}} {}

		template<typename U> requires std::is_constructible_v<T, U>
		// NOLINTNEXTLINE(*-explicit-constructor)
		constexpr result(__detail::success_type<U>&& value)
			: storage {success_tag {}, std::forward<U>(value.value)}, success_ {true} {}

		template<typename U> requires std::is_constructible_v<E, U>
		// NOLINTNEXTLINE(*-explicit-constructor)
		constexpr result(__detail::error_type<U>&& value)
			: storage {error_tag {}, std::forward<U>(value.value)}, success_ {false} {}

		constexpr result(const result& other) requires is_copyable = default;
		constexpr result(const result& other) : success_ {other.success_} {
			if (other.success_) {
				std::construct_at(&storage.value, other.storage.value);
			}
			else {
				std::construct_at(&storage.error, other.storage.error);
			}
		}

		constexpr result(result&& other) noexcept requires is_copyable = default;
		constexpr result(result&& other) noexcept : success_ {other.success_} {
			if (other.success_) {
				std::construct_at(&storage.value, std::move(other.storage.value));
			}
			else {
				std::construct_at(&storage.error, std::move(other.storage.error));
			}
		}

		constexpr result& operator=(const result& other) requires is_copyable = default;
		constexpr result& operator=(const result& other) { // NOLINT(*-unhandled-self-assignment)
			if (success_ && other.success_) {
				storage.value = other.storage.value;
			}
			else if (!success_ && !other.success_) {
				storage.error = other.storage.error;
			}
			else if (success_) {
				std::destroy_at(&storage.value);
				std::construct_at(&storage.error, other.storage.error);
				success_ = false;
			}
			else {
				std::destroy_at(&storage.error);
				std::construct_at(&storage.value, other.storage.value);
				success_ = true;
			}

			return *this;
		}

		constexpr result& operator=(result&& other) noexcept requires is_copyable = default;
		constexpr result& operator=(result&& other) noexcept { // NOLINT(*-unhandled-self-assignment)
			if (success_ && other.success_) {
				storage.value = std::move(other.storage.value);
			}
			else if (!success_ && !other.success_) {
				storage.error = std::move(other.storage.error);
			}
			else if (success_) {
				std::destroy_at(&storage.value);
				std::construct_at(&storage.error, std::move(other.storage.error));
				success_ = false;
			}
			else {
				std::destroy_at(&storage.error);
				std::construct_at(&storage.value, std::move(other.storage.value));
				success_ = true;
			}

			return *this;
		}

		template<typename U> requires std::is_constructible_v<T, U>
		constexpr result& operator=(__detail::success_type<U>&& value) {
			if (success_) {
				storage.value = std::forward<U>(value.value);
			}
			else {
				std::destroy_at(&storage.error);
				std::construct_at(&storage.value, std::forward<U>(value.value));
				success_ = true;
			}

			return *this;
		}

		template<typename U> requires std::is_constructible_v<E, U>
		constexpr result& operator=(__detail::error_type<U>&& value) {
			if (!success_) {
				storage.error = std::forward<U>(value.value);
			}
			else {
				std::destroy_at(&storage.value);
				std::construct_at(&storage.error, std::forward<U>(value.value));
				success_ = false;
			}

			return *this;
		}

		constexpr T& value() {
			return storage.value;
		}

		constexpr const T& value() const {
			return storage.value;
		}

		constexpr E& error() {
			return storage.error;
		}

		constexpr const E& error() const {
			return storage.error;
		}

		constexpr T* operator->() {
			return &storage.value;
		}

		constexpr const T* operator->() const {
			return &storage.value;
		}

		constexpr T& operator*() {
			return storage.value;
		}

		constexpr const T& operator*() const {
			return storage.value;
		}

		[[nodiscard]] constexpr bool has_value() const {
			return success_;
		}

		[[nodiscard]] constexpr bool has_error() const {
			return !success_;
		}

		constexpr explicit operator bool() const {
			return success_;
		}

		constexpr ~result() requires is_destructible = default;
		constexpr ~result() {
			if (success_) {
				std::destroy_at(&storage.value);
			}
			else {
				std::destroy_at(&storage.error);
			}
		}

	private:
		struct success_tag {};
		struct error_tag {};

		union storage {
			constexpr storage() {}

			template<typename... Args>
			constexpr storage(success_tag, Args&&... args) : value(std::forward<Args>(args)...) {}

			template<typename... Args>
			constexpr storage(error_tag, Args&&... args) : error(std::forward<Args>(args)...) {}

			constexpr ~storage() requires is_destructible = default;
			constexpr ~storage() {}

			T value;
			E error;
		} storage;

		bool success_ {};
	};
}
