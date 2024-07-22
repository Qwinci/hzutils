#pragma once
#include "hash.hpp"
#include "optional.hpp"
#include "vector.hpp"

namespace hz {
	template<typename K, typename T, typename Allocator, Hasher Hasher = fx_hasher>
	class unordered_map {
	public:
		constexpr explicit unordered_map(Allocator alloc) : table {std::move(alloc)} {}

		void insert(K key, const T& value) {
			Hasher hasher {};
			hash_impl<K, Hasher>::hash(hasher, key);
			auto hash = hasher.finish();

			if (table.empty()) {
				table.resize(8);
				table[hash % 8] = Slot {.key {std::move(key)}, .value {value}};
				++used;
				return;
			}

			auto load_factor = used * 100 / table.size();
			if (load_factor >= 70) {
				auto new_table = table.new_with_alloc();
				auto new_table_size = table.size() * 2;
				new_table.resize(new_table_size);

				for (auto& slot : table) {
					if (!slot.has_value()) {
						continue;
					}

					Hasher new_hasher {};
					hash_impl<K, Hasher>::hash(new_hasher, slot.value().key);
					auto slot_hash = new_hasher.finish();

					size_t bucket_index = slot_hash % new_table_size;
					while (true) {
						auto& bucket = new_table[bucket_index];
						if (!bucket.has_value()) {
							bucket = std::move(slot.value());
							break;
						}
						bucket_index = (bucket_index + 1) % new_table_size;
					}
				}

				table = std::move(new_table);
			}

			size_t table_size = table.size();
			size_t bucket_index = hash % table_size;
			while (true) {
				auto& bucket = table[bucket_index];
				if (!bucket.has_value() || bucket->key == key) {
					bucket = Slot {.key {std::move(key)}, .value {value}};
					++used;
					return;
				}
				bucket_index = (bucket_index + 1) % table_size;
			}
		}

		void insert(K key, T&& value) {
			Hasher hasher {};
			hash_impl<K, Hasher>::hash(hasher, key);
			auto hash = hasher.finish();

			if (table.empty()) {
				table.resize(8);
				table[hash % 8] = Slot {.key {std::move(key)}, .value {std::move(value)}};
				++used;
				return;
			}

			auto load_factor = used * 100 / table.size();
			if (load_factor >= 70) {
				auto new_table = table.new_with_alloc();
				auto new_table_size = table.size() * 2;
				new_table.resize(new_table_size);

				for (auto& slot : table) {
					if (!slot.has_value()) {
						continue;
					}

					Hasher new_hasher {};
					hash_impl<K, Hasher>::hash(new_hasher, slot.value().key);
					auto slot_hash = new_hasher.finish();

					size_t bucket_index = slot_hash % new_table_size;
					while (true) {
						auto& bucket = new_table[bucket_index];
						if (!bucket.has_value()) {
							bucket = std::move(slot.value());
							break;
						}
						bucket_index = (bucket_index + 1) % new_table_size;
					}
				}

				table = std::move(new_table);
			}

			size_t table_size = table.size();
			size_t bucket_index = hash % table_size;
			while (true) {
				auto& bucket = table[bucket_index];
				if (!bucket.has_value() || bucket->key == key) {
					bucket = Slot {.key {std::move(key)}, .value {std::move(value)}};
					if (!bucket.has_value()) {
						++used;
					}
					return;
				}
				bucket_index = (bucket_index + 1) % table_size;
			}
		}

		template<typename K2 = K> requires requires(K key, K2 comp) {
			key == comp;
		}
		void remove(K2 key) {
			auto table_size = table.size();
			if (!table_size) {
				return;
			}

			Hasher hasher {};
			hash_impl<K2, Hasher>::hash(hasher, key);
			auto hash = hasher.finish();

			size_t bucket_index = hash % table_size;
			size_t start = bucket_index;
			while (true) {
				auto& bucket = table[bucket_index];
				if (bucket && bucket.value().key == key) {
					bucket.reset();
					return;
				}
				bucket_index = (bucket_index + 1) % table_size;
				if (bucket_index == start) {
					return;
				}
			}
		}

		template<typename K2 = K> requires requires(K key, K2 comp) {
			key == comp;
		}
		[[nodiscard]] T* get(K2 key) {
			size_t table_size = table.size();
			if (!table_size) {
				return nullptr;
			}

			Hasher hasher {};
			hash_impl<K2, Hasher>::hash(hasher, key);
			auto hash = hasher.finish();

			size_t bucket_index = hash % table_size;
			size_t start = bucket_index;
			while (true) {
				auto& bucket = table[bucket_index];
				if (bucket && bucket.value().key == key) {
					return &bucket->value;
				}
				bucket_index = (bucket_index + 1) % table_size;
				if (bucket_index == start) {
					return nullptr;
				}
			}
		}

		[[nodiscard]] const T* get(K key) const {
			size_t table_size = table.size();
			if (!table_size) {
				return nullptr;
			}

			Hasher hasher {};
			hash_impl<K, Hasher>::hash(hasher, key);
			auto hash = hasher.finish();

			size_t bucket_index = hash % table_size;
			size_t start = bucket_index;
			while (true) {
				auto& bucket = table[bucket_index];
				if (bucket && bucket.value().key == key) {
					return &bucket->value;
				}
				bucket_index = (bucket_index + 1) % table_size;
				if (bucket_index == start) {
					return nullptr;
				}
			}
		}

		void clear() {
			table.clear();
			used = 0;
		}

	private:
		struct Slot {
			K key;
			T value;
		};
		vector<optional<Slot>, Allocator> table;
		size_t used {};
	};
}
