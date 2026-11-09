#pragma once

#include "config.hpp"
#include "string_utils.hpp"
#include "optional.hpp"

#include <stddef.h>

#if HZ_FREESTANDING
#include "type_traits.hpp"
#include "compare.hpp"
#include "limits.hpp"
#else
#include <type_traits>
#include <compare>
#include <limits>
#endif

namespace hz {
	template<typename T>
	class safe_int {
		using stype = std::make_signed_t<T>;
		using utype = std::make_unsigned_t<T>;

	public:
		static constexpr T min_value = std::numeric_limits<T>::min();
		static constexpr T max_value = std::numeric_limits<T>::max();
		static constexpr bool is_signed = std::numeric_limits<T>::is_signed;
		static constexpr int bits = std::numeric_limits<utype>::digits;

		constexpr safe_int() = default;
		constexpr safe_int(T value) : value_ {value} {}

		[[nodiscard]] constexpr optional<safe_int> overflowing_add(safe_int rhs) const {
			T res;
			bool overflow = false;
#if hz_has_builtin(__builtin_add_overflow)
			overflow = __builtin_add_overflow(value_, rhs.value_, &res);
#else
			if constexpr (is_signed) {
				if ((value_ >= 0 && rhs.value_ > max_value - value_) || (value_ < 0 && rhs.value_ < min_value - value_)) {
					overflow = true;
				}
				else {
					res = value_ + rhs.value_;
				}
			}
			else {
				if (rhs.value_ > max_value - value_) {
					overflow = true;
				}
				else {
					res = value_ + rhs.value_;
				}
			}
#endif

			if (overflow) {
				return nullopt;
			}

			return safe_int {res};
		}

		[[nodiscard]] constexpr optional<safe_int> overflowing_sub(safe_int rhs) const {
			T res;
			bool overflow = false;
#if hz_has_builtin(__builtin_sub_overflow)
			overflow = __builtin_sub_overflow(value_, rhs.value_, &res);
#else
			if constexpr (is_signed) {
				if ((rhs.value_ > 0 && value_ < min_value + rhs.value_) || (rhs.value_ < 0 && value_ > max_value + rhs.value_)) {
					overflow = true;
				}
				else {
					res = value_ - rhs.value_;
				}
			}
			else {
				if (value_ < rhs.value_) {
					overflow = true;
				}
				else {
					res = value_ - rhs.value_;
				}
			}
#endif

			if (overflow) {
				return nullopt;
			}

			return safe_int {res};
		}

		[[nodiscard]] constexpr optional<safe_int> overflowing_mul(safe_int rhs) const {
			T res;
			bool overflow = false;
#if hz_has_builtin(__builtin_mul_overflow)
			overflow = __builtin_mul_overflow(value_, rhs.value_, &res);
#else
			if (value_ == 0) {
				return {0};
			}

			if constexpr (is_signed) {
				if ((value_ >= 0 && rhs.value_ >= 0 && rhs.value_ > max_value / value_) ||
				    (value_ >= 0 && rhs.value_ < 0 && rhs.value_ < min_value / value_) ||
				    (value_ < 0 && rhs.value_ >= 0 && rhs.value_ > min_value / value_) ||
				    (value_ < 0 && rhs.value_ < 0 && rhs.value_ < max_value / value_)) {

					overflow = true;
				}
				else {
					res = value_ * rhs.value_;
				}
			}
			else {
				if (rhs.value_ > max_value / value_) {
					overflow = true;
				}
				else {
					res = value_ * rhs.value_;
				}
			}
#endif

			if (overflow) {
				return nullopt;
			}

			return safe_int {res};
		}

		[[nodiscard]] constexpr optional<safe_int> overflowing_div(safe_int rhs) const {
			if (rhs.value_ == 0) {
				return nullopt;
			}

			T res;
			if constexpr (is_signed) {
				if (value_ == min_value && rhs.value_ == -1) {
					return nullopt;
				}
				else {
					res = value_ / rhs.value_;
				}
			}
			else {
				res = value_ / rhs.value_;
			}

			return safe_int {res};
		}

		[[nodiscard]] constexpr optional<safe_int> overflowing_shl(safe_int rhs) const {
			if constexpr (is_signed) {
				if (rhs.value_ < 0 || rhs.value_ >= bits) {
					return nullopt;
				}
			}
			else {
				if (rhs.value_ >= bits) {
					return nullopt;
				}
			}

			T res = value_ << rhs.value_;
			return safe_int {res};
		}

		[[nodiscard]] constexpr optional<safe_int> overflowing_shr(safe_int rhs) const {
			if constexpr (is_signed) {
				if (rhs.value_ < 0 || rhs.value_ >= bits) {
					return nullopt;
				}
			}
			else {
				if (rhs.value_ >= bits) {
					return nullopt;
				}
			}

			T res = static_cast<T>(static_cast<utype>(value_) >> rhs.value_);
			return safe_int {res};
		}

		[[nodiscard]] constexpr optional<safe_int> overflowing_sar(safe_int rhs) const {
			if constexpr (is_signed) {
				if (rhs.value_ < 0 || rhs.value_ >= bits) {
					return nullopt;
				}
			}
			else {
				if (rhs.value_ >= bits) {
					return nullopt;
				}
			}

			T res = static_cast<T>(static_cast<stype>(value_) >> rhs.value_);
			return safe_int {res};
		}

		[[nodiscard]] constexpr safe_int saturating_add(safe_int rhs) const {
			auto res = overflowing_add(rhs);
			if (res) {
				return *res;
			}

			if constexpr (is_signed) {
				return {rhs.value_ < 0 ? min_value : max_value};
			}
			else {
				return {max_value};
			}
		}

		[[nodiscard]] constexpr safe_int saturating_sub(safe_int rhs) const {
			auto res = overflowing_sub(rhs);
			if (res) {
				return *res;
			}

			if constexpr (is_signed) {
				return {rhs.value_ > 0 ? min_value : max_value};
			}
			else {
				return {min_value};
			}
		}

		[[nodiscard]] constexpr safe_int saturating_mul(safe_int rhs) const {
			auto res = overflowing_mul(rhs);
			if (res) {
				return *res;
			}

			if constexpr (is_signed) {
				return {(value_ < 0) != (rhs.value_ < 0) ? min_value : max_value};
			}
			else {
				return {max_value};
			}
		}

		[[nodiscard]] constexpr safe_int saturating_div(safe_int rhs) const {
			auto res = overflowing_div(rhs);
			if (res) {
				return *res;
			}
			return {max_value};
		}

		[[nodiscard]] constexpr safe_int saturating_shl(safe_int rhs) const {
			auto res = overflowing_shl(rhs);
			if (res) {
				return *res;
			}
			return {max_value};
		}

		[[nodiscard]] constexpr safe_int saturating_shr(safe_int rhs) const {
			auto res = overflowing_shr(rhs);
			if (res) {
				return *res;
			}
			return {0};
		}

		[[nodiscard]] constexpr safe_int saturating_sar(safe_int rhs) const {
			auto res = overflowing_sar(rhs);
			if (res) {
				return *res;
			}
			return {0};
		}

		[[nodiscard]] constexpr safe_int wrapping_add(safe_int rhs) const {
			return {static_cast<T>(static_cast<utype>(value_) + static_cast<utype>(rhs.value_))};
		}

		[[nodiscard]] constexpr safe_int wrapping_sub(safe_int rhs) const {
			return {static_cast<T>(static_cast<utype>(value_) - static_cast<utype>(rhs.value_))};
		}

		[[nodiscard]] constexpr safe_int wrapping_mul(safe_int rhs) const {
			if constexpr (bits == 16) {
				return {static_cast<T>(static_cast<unsigned int>(value_) * static_cast<unsigned int>(rhs.value_))};
			}
			else {
				return {static_cast<T>(static_cast<utype>(value_) * static_cast<utype>(rhs.value_))};
			}
		}

		[[nodiscard]] constexpr safe_int negate() const {
			return {static_cast<T>(~static_cast<utype>(value_) + 1)};
		}

		[[nodiscard]] constexpr safe_int rotl(safe_int rhs) const {
			auto s = static_cast<unsigned int>(static_cast<utype>(rhs.value_)) & (bits - 1);

			if (!s) {
				return *this;
			}

			auto v = static_cast<utype>(value_);
			return {static_cast<T>(v << s | (v >> (bits - s)))};
		}

		[[nodiscard]] constexpr safe_int rotr(safe_int rhs) const {
			auto s = static_cast<unsigned int>(static_cast<utype>(rhs.value_)) & (bits - 1);

			if (!s) {
				return *this;
			}

			auto v = static_cast<utype>(value_);
			return {static_cast<T>(v >> s | (v << (bits - s)))};
		}

		[[nodiscard]] constexpr safe_int min(safe_int rhs) const {
			return {value_ < rhs.value_ ? value_ : rhs.value_};
		}

		[[nodiscard]] constexpr safe_int max(safe_int rhs) const {
			return {value_ > rhs.value_ ? value_ : rhs.value_};
		}

		[[nodiscard]] constexpr safe_int clamp(safe_int min, safe_int max) {
			return {value_ < min.value_ ? min.value_ : (value_ > max.value_ ? max.value_ : value_)};
		}

		[[nodiscard]] constexpr safe_int abs() const {
			if constexpr (is_signed) {
				if (value_ >= 0) {
					return *this;
				}

				return {static_cast<T>(~static_cast<utype>(value_) + 1)};
			}
			else {
				return *this;
			}
		}

		constexpr safe_int operator+(safe_int rhs) const {
			return wrapping_add(rhs);
		}

		constexpr safe_int& operator+=(safe_int rhs) {
			*this = *this + rhs;
			return *this;
		}

		constexpr safe_int operator-(safe_int rhs) const {
			return wrapping_sub(rhs);
		}

		constexpr safe_int& operator-=(safe_int rhs) {
			*this = *this - rhs;
			return *this;
		}

		constexpr safe_int operator*(safe_int rhs) const {
			return wrapping_mul(rhs);
		}

		constexpr safe_int& operator*=(safe_int rhs) {
			*this = *this * rhs;
			return *this;
		}

		constexpr safe_int operator/(safe_int rhs) const {
			if (rhs.value_ == 0) {
				return {0};
			}
			return saturating_div(rhs);
		}

		constexpr safe_int& operator/=(safe_int rhs) {
			*this = *this / rhs;
			return *this;
		}

		constexpr safe_int operator<<(safe_int rhs) const {
			return saturating_shl(rhs);
		}

		constexpr safe_int& operator<<=(safe_int rhs) {
			*this = *this << rhs;
			return *this;
		}

		constexpr safe_int operator>>(safe_int rhs) const {
			if constexpr (is_signed) {
				return saturating_sar(rhs);
			}
			else {
				return saturating_shr(rhs);
			}
		}

		constexpr safe_int& operator>>=(safe_int rhs) {
			*this = *this >> rhs;
			return *this;
		}

		constexpr auto operator<=>(const safe_int& other) const = default;
		constexpr bool operator==(const safe_int& other) const = default;

		template<typename U> requires(!std::is_same_v<U, safe_int>)
		constexpr auto operator<=>(U other) const {
			return value_ <=> static_cast<T>(other);
		}

		template<typename U> requires(!std::is_same_v<U, safe_int>)
		constexpr bool operator==(U other) const {
			return value_ == static_cast<T>(other);
		}

		[[nodiscard]] constexpr T value() const {
			return value_;
		}

		constexpr operator T() const {
			return value_;
		}

	public:
		T value_ {};
	};

	enum class str_to_int_status {
		success,
		value_too_large
	};

	template<typename T, typename C>
	struct str_to_int_result {
		T value;
		str_to_int_status status;
		const C* end;
	};

	template<typename T, int Base, typename C>
	constexpr str_to_int_result<T, C> str_to_int(const C* str, size_t size) {
		str_to_int_result<T, C> result {};

		if (!size) {
			result.end = str;
			return result;
		}

		const C* end_ptr = str + size;

		if (str == end_ptr) {
			result.end = str;
			return result;
		}

		safe_int<T> value;

		if constexpr (Base == 2) {
			for (; str != end_ptr; ++str) {
				C ch = *str;
				if (ch != C('0') && ch != C('1')) {
					break;
				}

				if (auto res = value.overflowing_mul(2)) {
					value = *res;

					if ((res = value.overflowing_add(ch - C('0')))) {
						value = *res;
					}
					else {
						result.status = str_to_int_status::value_too_large;
					}
				}
				else {
					result.status = str_to_int_status::value_too_large;
				}
			}
		}
		else if (Base == 8) {
			for (; str != end_ptr; ++str) {
				C ch = *str;
				if (ch < C('0') || ch > C('7')) {
					break;
				}

				if (auto res = value.overflowing_mul(8)) {
					value = *res;

					if ((res = value.overflowing_add(ch - C('0')))) {
						value = *res;
					}
					else {
						result.status = str_to_int_status::value_too_large;
					}
				}
				else {
					result.status = str_to_int_status::value_too_large;
				}
			}
		}
		else if (Base == 10) {
			for (; str != end_ptr; ++str) {
				C ch = *str;
				if (ch < C('0') || ch > C('9')) {
					break;
				}

				if (auto res = value.overflowing_mul(10)) {
					value = *res;

					if ((res = value.overflowing_add(ch - C('0')))) {
						value = *res;
					}
					else {
						result.status = str_to_int_status::value_too_large;
					}
				}
				else {
					result.status = str_to_int_status::value_too_large;
				}
			}
		}
		else {
			for (; str != end_ptr; ++str) {
				C ch = *str;

				int val;
				if (ch >= C('0') && ch <= C('9')) {
					val = ch - C('0');
				}
				else {
					ch = to_lower_ascii(ch);
					if (ch >= C('a') && ch <= C('f')) {
						val = ch - C('a') + 10;
					}
					else {
						break;
					}
				}

				if (auto res = value.overflowing_mul(16)) {
					value = *res;

					if ((res = value.overflowing_add(static_cast<T>(val)))) {
						value = *res;
					}
					else {
						result.status = str_to_int_status::value_too_large;
					}
				}
				else {
					result.status = str_to_int_status::value_too_large;
				}
			}
		}

		result.value = value;
		result.end = str;
		return result;
	}
}
