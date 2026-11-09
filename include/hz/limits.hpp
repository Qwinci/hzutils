#pragma once

#include <limits.h>
#include <stdint.h>

namespace std {
	template<typename __T>
	struct numeric_limits;

	template<>
	struct numeric_limits<bool> {
		static constexpr bool is_signed = false;
		static constexpr int digits = 1;

		static constexpr bool min() noexcept {
			return false;
		}

		static constexpr bool max() noexcept {
			return true;
		}
	};

	template<>
	struct numeric_limits<char> {
#ifdef __CHAR_UNSIGNED__
		static constexpr bool is_signed = false;
#else
		static constexpr bool is_signed = false;
#endif
		static constexpr int digits = CHAR_BIT - is_signed;

		static constexpr char min() noexcept {
#ifdef __CHAR_UNSIGNED__
			return 0;
#else
			return SCHAR_MIN;
#endif
		}

		static constexpr char max() noexcept {
#ifdef __CHAR_UNSIGNED__
			return UCHAR_MAX;
#else
			return SCHAR_MAX;
#endif
		}
	};

	template<>
	struct numeric_limits<signed char> {
		static constexpr bool is_signed = true;
		static constexpr int digits = CHAR_BIT - 1;

		static constexpr signed char min() noexcept {
			return SCHAR_MIN;
		}

		static constexpr signed char max() noexcept {
			return SCHAR_MAX;
		}
	};

	template<>
	struct numeric_limits<unsigned char> {
		static constexpr bool is_signed = false;
		static constexpr int digits = CHAR_BIT;

		static constexpr unsigned char min() noexcept {
			return 0;
		}

		static constexpr unsigned char max() noexcept {
			return UCHAR_MAX;
		}
	};

	template<>
	struct numeric_limits<wchar_t> {
#if defined(__WCHAR_UNSIGNED__) || defined(_MSC_VER)
		static constexpr bool is_signed = false;
#else
		static constexpr bool is_signed = true;
#endif
		static constexpr int digits = CHAR_BIT * sizeof(wchar_t) - is_signed;

		static constexpr wchar_t min() noexcept {
			return WCHAR_MIN;
		}

		static constexpr wchar_t max() noexcept {
			return WCHAR_MAX;
		}
	};

	template<>
	struct numeric_limits<char8_t> {
		static constexpr bool is_signed = false;
		static constexpr int digits = CHAR_BIT;

		static constexpr char8_t min() noexcept {
			return 0;
		}

		static constexpr char8_t max() noexcept {
			return UCHAR_MAX;
		}
	};

	template<>
	struct numeric_limits<char16_t> {
		static constexpr bool is_signed = false;
		static constexpr int digits = CHAR_BIT * sizeof(char16_t);

		static constexpr char16_t min() noexcept {
			return 0;
		}

		static constexpr char16_t max() noexcept {
			return UINT_LEAST16_MAX;
		}
	};

	template<>
	struct numeric_limits<char32_t> {
		static constexpr bool is_signed = false;
		static constexpr int digits = CHAR_BIT * sizeof(char32_t);

		static constexpr char32_t min() noexcept {
			return 0;
		}

		static constexpr char32_t max() noexcept {
			return UINT_LEAST32_MAX;
		}
	};

	template<>
	struct numeric_limits<short> {
		static constexpr bool is_signed = true;
		static constexpr int digits = CHAR_BIT * sizeof(short) - 1;

		static constexpr short min() noexcept {
			return SHRT_MIN;
		}

		static constexpr short max() noexcept {
			return SHRT_MAX;
		}
	};

	template<>
	struct numeric_limits<unsigned short> {
		static constexpr bool is_signed = false;
		static constexpr int digits = CHAR_BIT * sizeof(short);

		static constexpr unsigned short min() noexcept {
			return 0;
		}

		static constexpr unsigned short max() noexcept {
			return USHRT_MAX;
		}
	};

	template<>
	struct numeric_limits<int> {
		static constexpr bool is_signed = true;
		static constexpr int digits = CHAR_BIT * sizeof(int) - 1;

		static constexpr int min() noexcept {
			return INT_MIN;
		}

		static constexpr int max() noexcept {
			return INT_MAX;
		}
	};

	template<>
	struct numeric_limits<unsigned int> {
		static constexpr bool is_signed = false;
		static constexpr int digits = CHAR_BIT * sizeof(int);

		static constexpr unsigned int min() noexcept {
			return 0;
		}

		static constexpr unsigned int max() noexcept {
			return UINT_MAX;
		}
	};

	template<>
	struct numeric_limits<long> {
		static constexpr bool is_signed = true;
		static constexpr int digits = CHAR_BIT * sizeof(long) - 1;

		static constexpr long min() noexcept {
			return LONG_MIN;
		}

		static constexpr long max() noexcept {
			return LONG_MAX;
		}
	};

	template<>
	struct numeric_limits<unsigned long> {
		static constexpr bool is_signed = false;
		static constexpr int digits = CHAR_BIT * sizeof(long);

		static constexpr unsigned long min() noexcept {
			return 0;
		}

		static constexpr unsigned long max() noexcept {
			return ULONG_MAX;
		}
	};

	template<>
	struct numeric_limits<long long> {
		static constexpr bool is_signed = true;
		static constexpr int digits = CHAR_BIT * sizeof(long long) - 1;

		static constexpr long long min() noexcept {
			return LLONG_MIN;
		}

		static constexpr long long max() noexcept {
			return LLONG_MAX;
		}
	};

	template<>
	struct numeric_limits<unsigned long long> {
		static constexpr bool is_signed = false;
		static constexpr int digits = CHAR_BIT * sizeof(long long);

		static constexpr unsigned long long min() noexcept {
			return 0;
		}

		static constexpr unsigned long long max() noexcept {
			return ULLONG_MAX;
		}
	};
}
