#pragma once

#include "config.hpp"
#include "hash_impl.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	template<typename T1, typename T2>
	struct pair {
		T1 first;
		T2 second;
	};

	template<typename T1>
	struct pair<T1, void> {
		T1 first;
	};

	template<typename T1, typename T2, hasher Hasher>
	struct hash_impl<pair<T1, T2>, Hasher> {
		static constexpr void hash(const pair<T1, T2>& self, Hasher& hasher) {
			hasher.hash(self.first);
			hasher.hash(self.second);
		}
	};

	template<typename T1, hasher Hasher>
	struct hash_impl<pair<T1, void>, Hasher> {
		static constexpr void hash(const pair<T1, void>& self, Hasher& hasher) {
			hasher.hash(self.first);
		}
	};
}
