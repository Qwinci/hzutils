#pragma once

#include "config.hpp"

#include <stdint.h>

#include "hash_impl.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	class fnv_hasher32 : public hasher_base<fnv_hasher32> {
	public:
		using hash_type = uint32_t;

		constexpr explicit fnv_hasher32(uint32_t seed = 0) {
			value ^= seed;
		}

		template<typename C> requires(sizeof(C) == 1)
		constexpr void hash_bytes(const C* bytes, size_t size) {
			for (size_t i = 0; i < size; ++i) {
				value ^= static_cast<unsigned char>(bytes[i]);
				value *= prime;
			}
		}

		template<typename C> requires(sizeof(C) > 1)
		constexpr void hash_bytes(const C* bytes, size_t size) {
			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < size; ++i) {
					auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(C)>, bytes[i]);
					hash_bytes(arr.bytes, sizeof(arr.bytes));
				}
			}
			else {
				hash_bytes(reinterpret_cast<const unsigned char*>(bytes), size * sizeof(C));
			}
		}

		[[nodiscard]] constexpr uint32_t finish() const {
			return value;
		}

	private:
		static constexpr uint32_t prime = 0x1000193;
		static constexpr uint32_t offset = 0x811C9DC5;

		uint32_t value {offset};
	};

	class fnv_hasher64 : public hasher_base<fnv_hasher64> {
	public:
		using hash_type = uint64_t;

		constexpr explicit fnv_hasher64(uint64_t seed = 0) {
			value ^= seed;
		}

		template<typename C> requires(sizeof(C) == 1)
		constexpr void hash_bytes(const C* bytes, size_t size) {
			for (size_t i = 0; i < size; ++i) {
				value ^= static_cast<unsigned char>(bytes[i]);
				value *= prime;
			}
		}

		template<typename C> requires(sizeof(C) > 1)
		constexpr void hash_bytes(const C* bytes, size_t size) {
			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < size; ++i) {
					auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(C)>, bytes[i]);
					hash_bytes(arr.bytes, sizeof(arr.bytes));
				}
			}
			else {
				hash_bytes(reinterpret_cast<const unsigned char*>(bytes), size * sizeof(C));
			}
		}

		[[nodiscard]] constexpr uint64_t finish() const {
			return value;
		}

	private:
		static constexpr uint64_t prime = 0x100000001B3;
		static constexpr uint64_t offset = 0xCBF29CE484222325;

		uint64_t value {offset};
	};
}
