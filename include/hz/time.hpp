#pragma once

#include "config.hpp"

#include <stdint.h>

#if HZ_FREESTANDING
#include "compare.hpp"
#include "type_traits.hpp"
#else
#include <compare>
#include <type_traits>
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace hz::time {
	namespace __detail {
		template<int64_t To, int64_t From>
		constexpr int64_t rescale(int64_t value) {
			if constexpr (To >= From) {
				static_assert(To % From == 0);
				return value * (To / From);
			}
			else {
				static_assert(From % To == 0);
				return value / (From / To);
			}
		}
	}

	template<int64_t Scale>
	class time_type {
	public:
		static constexpr int64_t scale = Scale;

		constexpr time_type() = default;
		constexpr explicit time_type(int64_t value) : value_ {value} {}

		constexpr bool operator==(const time_type& other) const = default;
		constexpr auto operator<=>(const time_type& other) const = default;

		template<int64_t OtherScale>
		constexpr bool operator==(time_type<OtherScale> other) const {
			using __detail::rescale;
			constexpr int64_t fine = fine_scale<OtherScale>();
			return rescale<fine, Scale>(value_) == rescale<fine, OtherScale>(other.value_);
		}

		template<int64_t OtherScale>
		constexpr auto operator<=>(time_type<OtherScale> other) const {
			using __detail::rescale;
			constexpr int64_t fine = fine_scale<OtherScale>();
			return rescale<fine, Scale>(value_) <=> rescale<fine, OtherScale>(other.value_);
		}

		template<int64_t OtherScale>
		constexpr auto operator+(time_type<OtherScale> rhs) const {
			using __detail::rescale;
			constexpr int64_t fine = fine_scale<OtherScale>();
			return time_type<fine> {rescale<fine, Scale>(value_) + rescale<fine, OtherScale>(rhs.value_)};
		}

		template<int64_t OtherScale>
		constexpr time_type& operator+=(time_type<OtherScale> rhs) {
			*this = *this + rhs;
			return *this;
		}

		constexpr time_type operator+(int64_t rhs) const {
			return time_type {value_ + rhs};
		}

		constexpr time_type& operator+=(int64_t rhs) {
			*this = *this + rhs;
			return *this;
		}

		template<int64_t OtherScale>
		constexpr auto operator-(time_type<OtherScale> rhs) const {
			using __detail::rescale;
			constexpr int64_t fine = fine_scale<OtherScale>();
			return time_type<fine> {rescale<fine, Scale>(value_) - rescale<fine, OtherScale>(rhs.value_)};
		}

		template<int64_t OtherScale>
		constexpr time_type& operator-=(time_type<OtherScale> rhs) {
			*this = *this - rhs;
			return *this;
		}

		constexpr time_type operator-(int64_t rhs) const {
			return time_type {value_ - rhs};
		}

		constexpr time_type& operator-=(int64_t rhs) {
			*this = *this - rhs;
			return *this;
		}

		constexpr time_type operator*(int64_t rhs) const {
			return time_type {value_ * rhs};
		}

		constexpr time_type& operator*=(int64_t rhs) {
			*this = *this * rhs;
			return *this;
		}

		template<int64_t OtherScale>
		constexpr int64_t operator/(time_type<OtherScale> rhs) const {
			using __detail::rescale;
			constexpr int64_t fine = fine_scale<OtherScale>();
			return rescale<fine, Scale>(value_) / rescale<fine, OtherScale>(rhs.value_);
		}

		constexpr time_type operator/(int64_t rhs) const {
			return time_type {value_ / rhs};
		}

		constexpr time_type& operator/=(int64_t rhs) {
			*this = *this / rhs;
			return *this;
		}

		template<int64_t OtherScale>
		constexpr explicit(OtherScale < Scale) operator time_type<OtherScale>() const {
			using __detail::rescale;
			return time_type<OtherScale> {rescale<OtherScale, Scale>(value_)};
		}

		[[nodiscard]] constexpr time_type<1> seconds() const {
			return static_cast<time_type<1>>(*this);
		}

		[[nodiscard]] constexpr time_type<1000> milliseconds() const {
			return static_cast<time_type<1000>>(*this);
		}

		[[nodiscard]] constexpr time_type<1000000> microseconds() const {
			return static_cast<time_type<1000000>>(*this);
		}

		[[nodiscard]] constexpr time_type<1000000000> nanoseconds() const {
			return static_cast<time_type<1000000000>>(*this);
		}

		[[nodiscard]] constexpr time_type<1000000000000> picoseconds() const {
			return static_cast<time_type<1000000000000>>(*this);
		}

		[[nodiscard]] constexpr int64_t value() const {
			return value_;
		}

	private:
		template<int64_t>
		friend class time_type;

		template<int64_t OtherScale>
		static constexpr int64_t fine_scale() {
			return Scale < OtherScale ? OtherScale : Scale;
		}

		int64_t value_ {};
	};

	using seconds = time_type<1>;
	using milliseconds = time_type<1000>;
	using microseconds = time_type<1000000>;
	using nanoseconds = time_type<1000000000>;
	using picoseconds = time_type<1000000000000>;

	class fraction {
	public:
		constexpr fraction() = default;

		constexpr fraction(uint64_t num, uint64_t den, bool round_up = false) {
			whole = num / den;
			uint64_t rem = num % den;

			// (rem << 64) / den
			for (int i = 0; i < 64; ++i) {
				bool carry = rem >> 63;
				rem <<= 1;
				frac <<= 1;
				if (carry || rem >= den) {
					rem -= den;
					frac |= 1;
				}
			}

			if (round_up && rem != 0) {
				++frac;
			}
		}

		// Fraction used to convert ticks to ns.
		static constexpr fraction to_ns(uint64_t freq, bool round_up = false) {
			// ns_in_s / freq = nanoseconds in one tick
			return {nanoseconds::scale, freq, round_up};
		}

		// Fraction used to convert ns to ticks.
		static constexpr fraction to_ticks(uint64_t freq, bool round_up = false) {
			// freq / ns_in_s = ticks in one nanosecond
			return {freq, nanoseconds::scale, round_up};
		}

		// Returns the rounded down result of fraction * value.
		// Note: Use only when the fraction was built with round_up = false.
		constexpr uint64_t operator*(uint64_t value) const {
			return value * whole + mul_hi(value, frac);
		}

		// Returns the rounded up result of fraction * value.
		// Note: Use only when the fraction was built with round_up = true.
		constexpr uint64_t mul_round_up(uint64_t value) const {
			return value * whole + mul_hi(value, frac) + static_cast<uint64_t>(value * frac != 0);
		}

		friend constexpr uint64_t operator*(uint64_t lhs, fraction rhs) {
			return rhs * lhs;
		}

	private:
		static constexpr uint64_t mul_hi(uint64_t a, uint64_t b) {
#ifdef __SIZEOF_INT128__
			return (static_cast<unsigned __int128>(a) * static_cast<unsigned __int128>(b)) >> 64;
#elif defined(_MSC_VER) && (defined(_M_AMD64) || defined(_M_ARM64))
			if (std::is_constant_evaluated()) {
				return mul_hi_impl(a, b);
			}
			else {
				return __umulh(a, b);
			}
#else
			return mul_hi_impl(a, b);
#endif
		}

		static constexpr uint64_t mul_hi_impl(uint64_t a, uint64_t b) {
			uint64_t a_low = a & 0xFFFFFFFF;
			uint64_t a_high = a >> 32;
			uint64_t b_low = b & 0xFFFFFFFF;
			uint64_t b_high = b >> 32;

			uint64_t low_low = a_low * b_low;
			uint64_t low_high = a_low * b_high;
			uint64_t high_low = a_high * b_low;
			uint64_t high_high = a_high * b_high;

			uint64_t middle = (low_low >> 32) + (low_high & 0xFFFFFFFF) + (high_low & 0xFFFFFFFF);
			return high_high + (low_high >> 32) + (high_low >> 32) + (middle >> 32);
		}

		uint64_t whole {};
		uint64_t frac {};
	};

	struct tick_fractions {
		constexpr tick_fractions() = default;
		constexpr explicit tick_fractions(uint64_t freq)
			: to_ns {fraction::to_ns(freq)}, to_ticks {fraction::to_ticks(freq)},
			to_ticks_ceil {fraction::to_ticks(freq, true)} {}

		fraction to_ns;
		fraction to_ticks;
		fraction to_ticks_ceil;
	};
}
