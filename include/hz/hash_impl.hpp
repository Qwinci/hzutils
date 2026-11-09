#pragma once

#include "config.hpp"
#include "memory_utils.hpp"

#include <stddef.h>

#if HZ_FREESTANDING
#include "concepts.hpp"
#include "type_traits.hpp"
#else
#include <concepts>
#include <type_traits>
#endif

namespace hz {
	namespace __detail {
		template<size_t N>
		struct byte_array {
			unsigned char bytes[N];
		};
	}

	template<typename Self>
	class hasher_base {
	public:
		constexpr Self& hash(const auto& value);

	protected:
		template<typename W, typename C> requires(sizeof(C) == 1)
		static constexpr W load_word(const C* bytes) {
			if (std::is_constant_evaluated()) {
				__detail::byte_array<sizeof(W)> tmp;

				for (size_t i = 0; i < sizeof(W); ++i) {
					tmp.bytes[i] = static_cast<unsigned char>(bytes[i]);
				}

				return __builtin_bit_cast(W, tmp);
			}
			else {
				W value;
				memory_copy<void>(&value, bytes, sizeof(W));
				return value;
			}
		}
	};

	template<typename T>
	concept hasher = requires(T& hasher, const unsigned char* bytes, size_t size) {
		typename T::hash_type;
		static_cast<hasher_base<T>&>(hasher);
		hasher.hash_bytes(bytes, size);
		{ hasher.finish() } -> std::same_as<typename T::hash_type>;
	};

	template<typename T, hasher Hasher>
	struct hash_impl;

	template<hasher Hasher>
	struct hash_impl<char, Hasher> {
		static constexpr void hash(char value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<signed char, Hasher> {
		static constexpr void hash(signed char value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<unsigned char, Hasher> {
		static constexpr void hash(unsigned char value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<short, Hasher> {
		static constexpr void hash(short value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<unsigned short, Hasher> {
		static constexpr void hash(unsigned short value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<int, Hasher> {
		static constexpr void hash(int value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<unsigned int, Hasher> {
		static constexpr void hash(unsigned int value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<long, Hasher> {
		static constexpr void hash(long value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<unsigned long, Hasher> {
		static constexpr void hash(unsigned long value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<long long, Hasher> {
		static constexpr void hash(long long value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<unsigned long long, Hasher> {
		static constexpr void hash(unsigned long long value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<bool, Hasher> {
		static constexpr void hash(bool value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<wchar_t, Hasher> {
		static constexpr void hash(wchar_t value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<char8_t, Hasher> {
		static constexpr void hash(char8_t value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<char16_t, Hasher> {
		static constexpr void hash(char16_t value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<hasher Hasher>
	struct hash_impl<char32_t, Hasher> {
		static constexpr void hash(char32_t value, Hasher& hasher) {
			auto arr = __builtin_bit_cast(__detail::byte_array<sizeof(value)>, value);
			hasher.hash_bytes(arr.bytes, sizeof(arr.bytes));
		}
	};

	template<typename T, hasher Hasher> requires std::is_pointer_v<T>
	struct hash_impl<T, Hasher> {
		static void hash(T value, Hasher& hasher) {
			hasher.hash_bytes(reinterpret_cast<const unsigned char*>(&value), sizeof(value));
		}
	};

	template<typename Self>
	constexpr Self& hasher_base<Self>::hash(const auto& value) {
		hash_impl<std::remove_cvref_t<decltype(value)>, Self>::hash(value, *static_cast<Self*>(this));
		return *static_cast<Self*>(this);
	}
}
