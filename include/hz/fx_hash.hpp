#pragma once

#include "config.hpp"

#include <stdint.h>

#include "hash_impl.hpp"
#include "bit.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	class fx_hasher32 : public hasher_base<fx_hasher32> {
	public:
		using hash_type = uint32_t;

		constexpr explicit fx_hasher32(uint32_t seed = 0) : hash_ {seed} {}

		constexpr void hash_word(uint32_t word) {
			hash_ = (hash_ + word) * 0x93D765DD;
		}

		template<typename C> requires(sizeof(C) == 1)
		constexpr void hash_bytes(const C* bytes, size_t size) {
			while (size >= 4) {
				uint32_t value = load_word<uint32_t>(bytes);
				hash_word(value);
				bytes += 4;
				size -= 4;
			}

			while (size >= 2) {
				uint16_t value = load_word<uint16_t>(bytes);
				hash_word(value);
				bytes += 2;
				size -= 2;
			}

			if (size) {
				hash_word(static_cast<unsigned char>(*bytes));
			}
		}

		template<typename C> requires(sizeof(C) > 1)
		constexpr void hash_bytes(const C* bytes, size_t size) {
			if (std::is_constant_evaluated()) {
				__detail::byte_array<8> chunk {};
				size_t len = 0;

				for (size_t i = 0; i < size; ++i) {
					auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(C)>, bytes[i]);

					for (auto byte : arr.bytes) {
						chunk.bytes[len++] = byte;
					}

					if (len == sizeof(chunk.bytes)) {
						hash_bytes(chunk.bytes, len);
						len = 0;
					}
				}

				hash_bytes(chunk.bytes, len);
			}
			else {
				hash_bytes(reinterpret_cast<const unsigned char*>(bytes), size * sizeof(C));
			}
		}

		[[nodiscard]] constexpr uint32_t finish() const {
			return rotl(hash_, 15);
		}

	private:
		uint32_t hash_ {};
	};

	class fx_hasher64 : public hasher_base<fx_hasher64> {
	public:
		using hash_type = uint64_t;

		constexpr explicit fx_hasher64(uint64_t seed = 0) : hash_ {seed} {}

		constexpr void hash_word(uint64_t word) {
			hash_ = (hash_ + word) * 0xF1357AEA2E62A9C5;
		}

		template<typename C> requires(sizeof(C) == 1)
		constexpr void hash_bytes(const C* bytes, size_t size) {
			while (size >= 8) {
				uint64_t value = load_word<uint64_t>(bytes);
				hash_word(value);
				bytes += 8;
				size -= 8;
			}

			while (size >= 4) {
				uint32_t value = load_word<uint32_t>(bytes);
				hash_word(value);
				bytes += 4;
				size -= 4;
			}

			while (size >= 2) {
				uint16_t value = load_word<uint16_t>(bytes);
				hash_word(value);
				bytes += 2;
				size -= 2;
			}

			if (size) {
				hash_word(static_cast<unsigned char>(*bytes));
			}
		}

		template<typename C> requires(sizeof(C) > 1)
		constexpr void hash_bytes(const C* bytes, size_t size) {
			if (std::is_constant_evaluated()) {
				__detail::byte_array<8> chunk {};
				size_t len = 0;

				for (size_t i = 0; i < size; ++i) {
					auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(C)>, bytes[i]);

					for (auto byte : arr.bytes) {
						chunk.bytes[len++] = byte;
					}

					if (len == sizeof(chunk.bytes)) {
						hash_bytes(chunk.bytes, len);
						len = 0;
					}
				}

				hash_bytes(chunk.bytes, len);
			}
			else {
				hash_bytes(reinterpret_cast<const unsigned char*>(bytes), size * sizeof(C));
			}
		}

		[[nodiscard]] constexpr uint64_t finish() const {
			return rotl(hash_, 26);
		}

	private:
		uint64_t hash_ {};
	};

#if UINTPTR_MAX == UINT32_MAX
	using fx_hasher = fx_hasher32;
#else
	using fx_hasher = fx_hasher64;
#endif
}
