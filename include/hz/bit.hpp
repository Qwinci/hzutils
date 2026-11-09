#pragma once

#include "config.hpp"

#include <stdint.h>

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	template<typename To, typename From>
	constexpr To bit_cast(const From& from) noexcept {
		return __builtin_bit_cast(To, from);
	}

	template<typename T>
	constexpr T byteswap(T value) noexcept {
		if constexpr (sizeof(T) == 1) {
			return value;
		}
		else if constexpr (sizeof(T) == 2) {
#if defined(_MSC_VER) && !defined(__clang__)
			auto v = static_cast<unsigned short>(value);
			return static_cast<T>((v & 0xFF) << 8 | (v >> 8));
#else
			return static_cast<T>(__builtin_bswap16(static_cast<unsigned short>(value)));
#endif
		}
		else if constexpr (sizeof(T) == 4) {
#if defined(_MSC_VER) && !defined(__clang__)
			auto v = static_cast<unsigned int>(value);
			return static_cast<T>(
				(v & 0xFF) << 24 |
				((v >> 8) & 0xFF) << 16 |
				((v >> 16) & 0xFF) << 8 |
				(v >> 24));
#else
			return static_cast<T>(__builtin_bswap32(static_cast<unsigned int>(value)));
#endif
		}
		else {
#if defined(_MSC_VER) && !defined(__clang__)
			auto v = static_cast<unsigned long long>(value);
			return static_cast<T>(
				(v & 0xFF) << 56 |
				((v >> 8) & 0xFF) << 48 |
				((v >> 16) & 0xFF) << 40 |
				((v >> 24) & 0xFF) << 32 |
				((v >> 32) & 0xFF) << 24 |
				((v >> 40) & 0xFF) << 16 |
				((v >> 48) & 0xFF) << 8 |
				(v >> 56));
#else
			return static_cast<T>(__builtin_bswap64(static_cast<unsigned long long>(value)));
#endif
		}
	}

	template<typename T>
	constexpr bool has_single_bit(T value) noexcept {
		return value && !(value & (value - 1));
	}

	template<typename T>
	constexpr int countl_zero(T value) noexcept {
		static_assert(T(-1) > T(0));

		constexpr int bits = static_cast<int>(sizeof(T)) * 8;

		if (value == 0) {
			return bits;
		}

#if defined(_MSC_VER) && !defined(__clang__)
		if (std::is_constant_evaluated()) {
			for (int i = bits - 1; i >= 0; --i) {
				if (value & (1ULL << i)) {
					return bits - 1 - i;
				}
			}

			__assume(false);
		}
#endif

		if constexpr (sizeof(T) == 1) {
#if defined(_MSC_VER) && !defined(__clang__)
			unsigned long index;
			_BitScanReverse(&index, static_cast<unsigned long>(value));
			return 31 - index - 24;
#else
			return __builtin_clz(value) - 24;
#endif
		}
		else if constexpr (sizeof(T) == 2) {
#if defined(_MSC_VER) && !defined(__clang__)
			unsigned long index;
			_BitScanReverse(&index, static_cast<unsigned long>(value));
			return 31 - index - 16;
#else
			return __builtin_clz(value) - 16;
#endif
		}
		else if constexpr (sizeof(T) == 4) {
#if defined(_MSC_VER) && !defined(__clang__)
			unsigned long index;
			_BitScanReverse(&index, static_cast<unsigned long>(value));
			return 31 - index;
#else
			return __builtin_clz(value);
#endif
		}
		else {
#if defined(_MSC_VER) && !defined(__clang__)
			auto v = static_cast<unsigned long long>(value);
#ifndef _M_IX86
			unsigned long index;
			_BitScanReverse64(&index, v);
			return 63 - index;
#else
			for (int i = 63; i >= 0; --i) {
				if (v & (1ULL << i)) {
					return 63 - i;
				}
			}

			__assume(false);
#endif
#else
			return __builtin_clzll(value);
#endif
		}
	}

	template<typename T>
	constexpr int countr_zero(T value) noexcept {
		static_assert(T(-1) > T(0));

		constexpr int bits = static_cast<int>(sizeof(T)) * 8;

		if (value == 0) {
			return bits;
		}

#if defined(_MSC_VER) && !defined(__clang__)
		if (std::is_constant_evaluated()) {
			for (int i = 0; i < bits; ++i) {
				if (value & (1ULL << i)) {
					return i;
				}
			}

			__assume(false);
		}
#endif

		if constexpr (sizeof(T) == 1) {
#if defined(_MSC_VER) && !defined(__clang__)
			unsigned long index;
			_BitScanForward(&index, static_cast<unsigned long>(value));
			return static_cast<int>(index);
#else
			return __builtin_ctz(value);
#endif
		}
		else if constexpr (sizeof(T) == 2) {
#if defined(_MSC_VER) && !defined(__clang__)
			unsigned long index;
			_BitScanForward(&index, static_cast<unsigned long>(value));
			return static_cast<int>(index);
#else
			return __builtin_ctz(value);
#endif
		}
		else if constexpr (sizeof(T) == 4) {
#if defined(_MSC_VER) && !defined(__clang__)
			unsigned long index;
			_BitScanForward(&index, static_cast<unsigned long>(value));
			return static_cast<int>(index);
#else
			return __builtin_ctz(value);
#endif
		}
		else {
#if defined(_MSC_VER) && !defined(__clang__)
			auto v = static_cast<unsigned long long>(value);
#ifndef _M_IX86
			unsigned long index;
			_BitScanForward64(&index, v);
			return static_cast<int>(index);
#else
			for (int i = 0; i < 64; ++i) {
				if (v & (1ULL << i)) {
					return i;
				}
			}

			__assume(false);
#endif
#else
			return __builtin_ctzll(value);
#endif
		}
	}

	template<typename T>
	constexpr int popcount(T value) noexcept {
		static_assert(T(-1) > T(0));

		constexpr int bits = static_cast<int>(sizeof(T)) * 8;
		(void) bits;

		if (value == 0) {
			return 0;
		}

#if defined(_MSC_VER) && !defined(__clang__)
		if (std::is_constant_evaluated()) {
			int count = 0;
			for (int i = 0; i < bits; ++i) {
				if (value & (1ULL << i)) {
					++count;
				}
			}

			return count;
		}
#endif

		if constexpr (sizeof(T) <= 4) {
#if defined(_MSC_VER) && !defined(__clang__)
			return static_cast<int>(__popcnt(value));
#else
			return __builtin_popcount(value);
#endif
		}
		else {
#if defined(_MSC_VER) && !defined(__clang__)
#ifndef _M_IX86
			return static_cast<int>(__popcnt64(value));
#else
			auto v = static_cast<unsigned long long>(value);
			int count = 0;
			for (int i = 0; i < 64; ++i) {
				if (v & (1ULL << i)) {
					++count;
				}
			}

			return count;
#endif
#else
			return __builtin_popcountll(value);
#endif
		}
	}

	template<typename T>
	constexpr int bit_width(T value) noexcept {
		static_assert(T(-1) > T(0));
		return static_cast<int>(sizeof(T)) * 8 - countl_zero(value);
	}

	template<typename T>
	constexpr T bit_ceil(T value) noexcept {
		if (value == 0) {
			return T {1};
		}

		return T {1} << bit_width(T(value - 1));
	}

	template<typename T>
	constexpr T bit_floor(T value) noexcept {
		if (value == 0) {
			return 0;
		}

		return T {1} << (bit_width(value) - 1);
	}

	template<typename T>
	constexpr T rotl(T value, int s) noexcept {
		constexpr int bits = static_cast<int>(sizeof(T)) * 8;

		if (!s) {
			return value;
		}

		return value << s | (value >> (bits - s));
	}

	template<typename T>
	constexpr T rotr(T value, int s) noexcept {
		constexpr int bits = static_cast<int>(sizeof(T)) * 8;

		if (!s) {
			return value;
		}

		return value >> s | (value << (bits - s));
	}

	enum class endian {
#if defined(_MSC_VER) && !defined(__clang__)
		little = 0,
		big = 1,
		native = little
#else
		little = __ORDER_LITTLE_ENDIAN__,
		big = __ORDER_BIG_ENDIAN__,
		native = __BYTE_ORDER__
#endif
	};

	template<endian To, endian From, typename T>
	constexpr T to_endian(T value) {
		if constexpr (To == From) {
			return value;
		}
		else {
			return byteswap(value);
		}
	}

	template<typename T>
	constexpr T to_le(T value) {
		return to_endian<endian::little, endian::native>(value);
	}

	template<typename T>
	constexpr T to_be(T value) {
		return to_endian<endian::big, endian::native>(value);
	}

	template<typename T>
	constexpr T from_le(T value) {
		return to_endian<endian::native, endian::little>(value);
	}

	template<typename T>
	constexpr T from_be(T value) {
		return to_endian<endian::native, endian::big>(value);
	}

	template<typename T, endian Endian>
	class integer {
	public:
		constexpr integer() = default;
		// NOLINTNEXTLINE
		constexpr integer(T value) : value_ {to_endian<Endian, endian::native>(value)} {}

		[[nodiscard]] constexpr T load() const {
			return to_endian<endian::native, Endian>(value_);
		}

		constexpr void store(T value) const {
			value_ = to_endian<Endian, endian::native>(value);
		}

		constexpr integer& operator=(T value) {
			store(value);
			return *this;
		}

		// NOLINTNEXTLINE
		constexpr operator T() const {
			return load();
		}

	private:
		T value_;
	};

	using le16 = integer<uint16_t, endian::little>;
	using le32 = integer<uint32_t, endian::little>;
	using le64 = integer<uint64_t, endian::little>;

	using be16 = integer<uint16_t, endian::big>;
	using be32 = integer<uint32_t, endian::big>;
	using be64 = integer<uint64_t, endian::big>;
}
