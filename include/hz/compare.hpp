#pragma once

namespace std {
	struct __CmpUnspecifiedParam {
		consteval __CmpUnspecifiedParam(int __zero) noexcept
#if __has_attribute(__enable_if__)
			__attribute__((__enable_if__(__zero == 0, "")))
#endif
		{
			(void) __zero;
		}
	};

	class strong_ordering {
		constexpr explicit strong_ordering(signed char __val) noexcept : __value {__val} {}

	public:
		static const strong_ordering less;
		static const strong_ordering equal;
		static const strong_ordering greater;

		friend constexpr bool operator==(strong_ordering, strong_ordering) noexcept = default;

		friend constexpr bool operator==(strong_ordering __lhs, __CmpUnspecifiedParam) noexcept {
			return __lhs.__value == 0;
		}

		friend constexpr bool operator<(strong_ordering __lhs, __CmpUnspecifiedParam) noexcept {
			return __lhs.__value < 0;
		}

		friend constexpr bool operator<=(strong_ordering __lhs, __CmpUnspecifiedParam) noexcept {
			return __lhs.__value <= 0;
		}

		friend constexpr bool operator>(strong_ordering __lhs, __CmpUnspecifiedParam) noexcept {
			return __lhs.__value > 0;
		}

		friend constexpr bool operator>=(strong_ordering __lhs, __CmpUnspecifiedParam) noexcept {
			return __lhs.__value >= 0;
		}

		friend constexpr bool operator<(__CmpUnspecifiedParam, strong_ordering __rhs) noexcept {
			return 0 < __rhs.__value;
		}

		friend constexpr bool operator<=(__CmpUnspecifiedParam, strong_ordering __rhs) noexcept {
			return 0 <= __rhs.__value;
		}

		friend constexpr bool operator>(__CmpUnspecifiedParam, strong_ordering __rhs) noexcept {
			return 0 > __rhs.__value;
		}

		friend constexpr bool operator>=(__CmpUnspecifiedParam, strong_ordering __rhs) noexcept {
			return 0 >= __rhs.__value;
		}

		friend constexpr strong_ordering operator<=>(strong_ordering __lhs, __CmpUnspecifiedParam) noexcept {
			return __lhs;
		}

		friend constexpr strong_ordering operator<=>(__CmpUnspecifiedParam, strong_ordering __rhs) noexcept {
			return __rhs < 0 ? strong_ordering::greater : (__rhs > 0 ? strong_ordering::less : __rhs);
		}

	private:
		signed char __value;
	};

	inline constexpr strong_ordering strong_ordering::less {-1};
	inline constexpr strong_ordering strong_ordering::equal {0};
	inline constexpr strong_ordering strong_ordering::greater {1};
}
