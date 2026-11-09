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
	struct nullopt_t {
		constexpr explicit nullopt_t(int) {}
	};

	inline constexpr nullopt_t nullopt {0};

	template<typename T>
	class optional {
		static constexpr bool is_copyable = std::is_trivially_copyable_v<T>;

	public:
		friend constexpr void swap(optional& a, optional& b) {
			using std::swap;

			if (a.has_value_ && b.has_value_) {
				swap(a.storage.value, b.storage.value);
			}
			else if (a.has_value_) {
				std::construct_at(&b.storage.value, std::move(a.storage.value));
				std::destroy_at(&a.storage.value);
				swap(a.has_value_, b.has_value_);
			}
			else if (b.has_value_) {
				std::construct_at(&a.storage.value, std::move(b.storage.value));
				std::destroy_at(&b.storage.value);
				swap(a.has_value_, b.has_value_);
			}
		}

		constexpr optional() noexcept = default;
		constexpr optional(nullopt_t) noexcept {}

		constexpr optional(const T& value) : storage {std::in_place, value}, has_value_ {true} {}

		constexpr optional(T&& value) : storage {std::in_place, std::move(value)}, has_value_ {true} {}

		constexpr optional(const optional& other) requires is_copyable = default;
		constexpr optional(const optional& other) : has_value_ {other.has_value_} {
			if (other.has_value_) {
				std::construct_at(&storage.value, other.storage.value);
			}
		}

		constexpr optional(optional&& other) noexcept requires is_copyable = default;
		constexpr optional(optional&& other) noexcept : has_value_ {other.has_value_} {
			if (other.has_value_) {
				std::construct_at(&storage.value, std::move(other.storage.value));
			}
		}

		constexpr optional& operator=(nullopt_t) noexcept {
			reset();
			return *this;
		}

		constexpr optional& operator=(const optional& other) requires is_copyable = default;
		constexpr optional& operator=(const optional& other) { // NOLINT(*-unhandled-self-assignment)
			if (has_value_ && other.has_value_) {
				storage.value = other.storage.value;
			}
			else if (other.has_value_) {
				std::construct_at(&storage.value, other.storage.value);
				has_value_ = true;
			}
			else if (has_value_) {
				std::destroy_at(&storage.value);
				has_value_ = false;
			}

			return *this;
		}

		constexpr optional& operator=(optional&& other) requires is_copyable = default;
		constexpr optional& operator=(optional&& other) { // NOLINT(*-unhandled-self-assignment)
			if (has_value_ && other.has_value_) {
				storage.value = std::move(other.storage.value);
			}
			else if (other.has_value_) {
				std::construct_at(&storage.value, std::move(other.storage.value));
				has_value_ = true;
			}
			else if (has_value_) {
				std::destroy_at(&storage.value);
				has_value_ = false;
			}

			return *this;
		}

		constexpr optional& operator=(const T& value) {
			if (has_value_) {
				storage.value = value;
			}
			else {
				std::construct_at(&storage.value, value);
				has_value_ = true;
			}

			return *this;
		}

		constexpr optional& operator=(T&& value) {
			if (has_value_) {
				storage.value = std::move(value);
			}
			else {
				std::construct_at(&storage.value, std::move(value));
				has_value_ = true;
			}

			return *this;
		}

		constexpr T& value() {
			return storage.value;
		}

		constexpr const T& value() const {
			return storage.value;
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
			return has_value_;
		}

		constexpr explicit operator bool() const {
			return has_value_;
		}

		constexpr void reset() {
			if (has_value_) {
				std::destroy_at(&storage.value);
				has_value_ = false;
			}
		}

		constexpr ~optional() requires std::is_trivially_destructible_v<T> = default;
		constexpr ~optional() {
			reset();
		}

	private:
		union storage {
			constexpr storage() {}

			template<typename... Args>
			constexpr storage(std::in_place_t, Args&&... args) : value(std::forward<Args>(args)...) {}

			constexpr ~storage() requires std::is_trivially_destructible_v<T> = default;
			constexpr ~storage() {}

			T value;
		} storage;

		bool has_value_ {};
	};
}
