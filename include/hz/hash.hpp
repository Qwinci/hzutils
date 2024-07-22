#pragma once
#include <stdint.h>
#include <stddef.h>
#include "bit.hpp"

namespace hz {
	struct fx_hasher32 {
		using hash_type = uint32_t;

		void hash(uint32_t word) {
			_hash = (rotl(_hash, 5) ^ word) * 0x9E3779B9;
		}

		void hash(const uint8_t* bytes, size_t size) {
			while (size >= 4) {
				uint32_t value;
				__builtin_memcpy(&value, bytes, 4);
				hash(value);
				bytes += 4;
				size -= 4;
			}

			while (size >= 2) {
				uint16_t value;
				__builtin_memcpy(&value, bytes, 2);
				hash(value);
				bytes += 2;
				size -= 2;
			}

			for (; size; --size, ++bytes) {
				hash(*bytes);
			}
		}

		[[nodiscard]] uint32_t finish() const {
			return _hash;
		}

	private:
		uint32_t _hash;
	};

	struct fx_hasher64 {
		using hash_type = uint64_t;

		void hash(uint64_t word) {
			_hash = (rotl(_hash, 5) ^ word) * 0x517CC1B727220A95;
		}

		void hash(const uint8_t* bytes, size_t size) {
			while (size >= 8) {
				uint64_t value;
				__builtin_memcpy(&value, bytes, 8);
				hash(value);
				bytes += 8;
				size -= 8;
			}

			while (size >= 4) {
				uint32_t value;
				__builtin_memcpy(&value, bytes, 4);
				hash(value);
				bytes += 4;
				size -= 4;
			}

			while (size >= 2) {
				uint16_t value;
				__builtin_memcpy(&value, bytes, 2);
				hash(value);
				bytes += 2;
				size -= 2;
			}

			for (; size; --size, ++bytes) {
				hash(*bytes);
			}
		}

		[[nodiscard]] uint64_t finish() const {
			return _hash;
		}

	private:
		uint64_t _hash;
	};

#if UINTPTR_MAX == UINT32_MAX
	using fx_hasher = fx_hasher32;
#else
	using fx_hasher = fx_hasher64;
#endif

	template<typename T>
	concept Hasher = requires(T hasher, const uint8_t* bytes) {
	hasher.hash(uint8_t {});
	hasher.hash(uint16_t {});
	hasher.hash(uint32_t {});
	hasher.hash(bytes, size_t {20});
	static_cast<typename T::hash_type>(hasher.finish());
};

	template<typename T, Hasher Hasher>
	struct hash_impl;

	template<typename Hasher>
	struct hash_impl<uint8_t, Hasher> {
		static void hash(Hasher& hasher, uint8_t value) {
			hasher.hash(value);
		}
	};

	template<typename Hasher>
	struct hash_impl<uint16_t, Hasher> {
		static void hash(Hasher& hasher, uint16_t value) {
			hasher.hash(value);
		}
	};

	template<typename Hasher>
	struct hash_impl<uint32_t, Hasher> {
		static void hash(Hasher& hasher, uint32_t value) {
			hasher.hash(value);
		}
	};

	template<typename Hasher>
	struct hash_impl<uint64_t, Hasher> {
		static void hash(Hasher& hasher, uint64_t value) {
			if constexpr (is_same_v<typename Hasher::hash_type, uint64_t>) {
				hasher.hash(value);
			}
			else {
				hasher.hash(static_cast<uint32_t>(value));
				hasher.hash(static_cast<uint32_t>(value >> 32));
			}
		}
	};

	template<typename Hasher>
	struct hash_impl<int8_t, Hasher> {
		static void hash(Hasher& hasher, int8_t value) {
			hasher.hash(value);
		}
	};

	template<typename Hasher>
	struct hash_impl<int16_t, Hasher> {
		static void hash(Hasher& hasher, int16_t value) {
			hasher.hash(value);
		}
	};

	template<typename Hasher>
	struct hash_impl<int32_t, Hasher> {
		static void hash(Hasher& hasher, int32_t value) {
			hasher.hash(value);
		}
	};

	template<typename Hasher>
	struct hash_impl<int64_t, Hasher> {
		static void hash(Hasher& hasher, int64_t value) {
			if constexpr (is_same_v<typename Hasher::hash_type, uint64_t>) {
				hasher.hash(value);
			}
			else {
				hasher.hash(static_cast<uint32_t>(value));
				hasher.hash(static_cast<uint32_t>(value >> 32));
			}
		}
	};

	template<typename T, typename Hasher>
	struct hash_impl<const T*, Hasher> {
		static void hash(Hasher& hasher, const T* ptr) {
			auto value = reinterpret_cast<uintptr_t>(ptr);
#if UINTPTR_MAX == UINT64_MAX
			if constexpr (is_same_v<typename Hasher::hash_type, uint64_t>) {
				hasher.hash(value);
			}
			else {
				hasher.hash(static_cast<uint32_t>(value));
				hasher.hash(static_cast<uint32_t>(value >> 32));
			}
#else
			hasher.hash(value);
#endif
		}
	};

	template<typename T, typename Hasher>
	struct hash_impl<T*, Hasher> {
		static void hash(Hasher& hasher, T* ptr) {
			auto value = reinterpret_cast<uintptr_t>(ptr);
#if UINTPTR_MAX == UINT64_MAX
			if constexpr (is_same_v<typename Hasher::hash_type, uint64_t>) {
				hasher.hash(value);
			}
			else {
				hasher.hash(static_cast<uint32_t>(value));
				hasher.hash(static_cast<uint32_t>(value >> 32));
			}
#else
			hasher.hash(value);
#endif
		}
	};
}
