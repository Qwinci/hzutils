#pragma once

#include "config.hpp"
#include "allocator.hpp"
#include "stl_allocator.hpp"
#include "memory_utils.hpp"
#include "bit_utils.hpp"
#include "hash_impl.hpp"
#include "fx_hash.hpp"
#include "bit.hpp"
#include "pair.hpp"

#include <stdint.h>
#include <stddef.h>

#if HZ_FREESTANDING
#include "initializer_list.hpp"
#include "type_traits.hpp"
#include "memory.hpp"
#include "utility.hpp"
#else
#include <initializer_list>
#include <type_traits>
#include <memory>
#include <utility>
#endif

#ifdef HZ_USE_SSE2
#include <emmintrin.h>
#endif

namespace hz {
	namespace __detail {
		inline constexpr uint8_t EMPTY = 0xFF;
		inline constexpr uint8_t TOMBSTONE = 0x80;

		template<size_t>
		struct group_info;

		template<>
		struct group_info<4> {
			alignas(4) static constexpr uint8_t empty[] {EMPTY, EMPTY, EMPTY, EMPTY};
			static constexpr uint32_t high = 0x80808080;
		};

		template<>
		struct group_info<8> {
			alignas(8) static constexpr uint8_t empty[] {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY};
			static constexpr uint64_t high = 0x8080808080808080;
		};

		template<>
		struct group_info<16> {
			alignas(16) static constexpr uint8_t empty[] {
				EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
				EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY
			};
		};

		struct bitmask_fallback {
			[[nodiscard]] constexpr int lowest_byte() const {
				return countr_zero(value) / 8;
			}

			[[nodiscard]] constexpr int highest_byte() const {
				return countl_zero(value) / 8;
			}

			constexpr void advance() {
				value &= value - 1;
			}

			constexpr void clear_below(unsigned int byte_offset) {
				value &= ~((uintptr_t(1) << (byte_offset * 8)) - 1);
			}

			constexpr explicit operator bool() const {
				return value != 0;
			}

			uintptr_t value;
		};

		struct group_fallback {
			static constexpr group_fallback load(const uint8_t* ctrl, size_t index) {
				uintptr_t value;

				if (std::is_constant_evaluated()) {
					value = 0;

					for (size_t i = 0; i < sizeof(uintptr_t); ++i) {
						value |= static_cast<uintptr_t>(ctrl[index + i]) << (i * 8);
					}
				}
				else {
					value = unaligned_load<uintptr_t>(ctrl + index);
				}

				return {from_le(value)};
			}

			[[nodiscard]] constexpr bitmask_fallback match_byte(uint8_t byte) const {
				return {match_bytes(value, byte)};
			}

			[[nodiscard]] constexpr bitmask_fallback match_full() const {
				return {~value & group_info<sizeof(uintptr_t)>::high};
			}

			[[nodiscard]] constexpr bitmask_fallback match_empty() const {
				return {match_bytes(value, EMPTY)};
			}

			[[nodiscard]] constexpr bitmask_fallback match_empty_or_tombstone() const {
				return {value & group_info<sizeof(uintptr_t)>::high};
			}

			uintptr_t value;
		};

#ifdef HZ_USE_SSE2
		struct bitmask_sse2 {
			[[nodiscard]] constexpr int lowest_byte() const {
				return countr_zero(static_cast<unsigned int>(value));
			}

			[[nodiscard]] constexpr int highest_byte() const {
				return countl_zero(static_cast<uint16_t>(value));
			}

			constexpr void advance() {
				value &= value - 1;
			}

			constexpr void clear_below(unsigned int byte_offset) {
				value &= ~((1 << byte_offset) - 1);
			}

			constexpr explicit operator bool() const {
				return value != 0;
			}

			int value;
		};

		struct group_sse2 {
			[[nodiscard]] static constexpr group_sse2 load(const uint8_t* ctrl, size_t index) {
				group_sse2 group;

				if (std::is_constant_evaluated()) {
					group = {};

					for (size_t i = 0; i < 8; ++i) {
						group.value_[0] |= static_cast<uint64_t>(ctrl[index + i]) << (i * 8);
					}

					for (size_t i = 0; i < 8; ++i) {
						group.value_[1] |= static_cast<uint64_t>(ctrl[index + 8 + i]) << (i * 8);
					}
				}
				else {
					__m128i value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ctrl + index));
					_mm_storeu_si128(reinterpret_cast<__m128i*>(group.value_), value);
				}

				return group;
			}

			[[nodiscard]] constexpr bitmask_sse2 match_byte(uint8_t byte) const {
				if (std::is_constant_evaluated()) {
					bitmask_sse2 res {};

					for (int i = 0; i < 16; ++i) {
						auto value = value_[i / 8];
						value >>= (i % 8) * 8;

						uint8_t got = value & 0xFF;
						if (got == byte) {
							res.value |= 1 << i;
						}
					}

					return res;
				}
				else {
					__m128i value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value_));
					int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(value, _mm_set1_epi8(static_cast<char>(byte))));
					return {mask};
				}
			}

			[[nodiscard]] constexpr bitmask_sse2 match_full() const {
				if (std::is_constant_evaluated()) {
					bitmask_sse2 res {};

					for (int i = 0; i < 16; ++i) {
						auto value = value_[i / 8];
						value >>= (i % 8) * 8;

						uint8_t got = value & 0xFF;
						if (!(got & 0x80)) {
							res.value |= 1 << i;
						}
					}

					return res;
				}
				else {
					__m128i value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value_));
					int mask = ~_mm_movemask_epi8(value) & 0xFFFF;
					return {mask};
				}
			}

			[[nodiscard]] constexpr bitmask_sse2 match_empty() const {
				if (std::is_constant_evaluated()) {
					bitmask_sse2 res {};

					for (int i = 0; i < 16; ++i) {
						auto value = value_[i / 8];
						value >>= (i % 8) * 8;

						uint8_t got = value & 0xFF;
						if (got == EMPTY) {
							res.value |= 1 << i;
						}
					}

					return res;
				}
				else {
					__m128i value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value_));
					int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(value, _mm_set1_epi8(static_cast<char>(EMPTY))));
					return {mask};
				}
			}

			[[nodiscard]] constexpr bitmask_sse2 match_empty_or_tombstone() const {
				if (std::is_constant_evaluated()) {
					bitmask_sse2 res {};

					for (int i = 0; i < 16; ++i) {
						auto value = value_[i / 8];
						value >>= (i % 8) * 8;

						uint8_t got = value & 0xFF;
						if (got & 0x80) {
							res.value |= 1 << i;
						}
					}

					return res;
				}
				else {
					__m128i value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(value_));
					int mask = _mm_movemask_epi8(value);
					return {mask};
				}
			}

			uint64_t value_[2];
		};
#endif
	}

	template<typename K, typename T, allocator<pair<K, T>> Allocator = stl_allocator<pair<K, T>>, hasher Hasher = fx_hasher>
	class unordered_map {
		using alloc_res = alloc_result<Allocator::infallible>;
		using insert_alloc_res = alloc_result<Allocator::infallible, bool>;

#ifdef HZ_USE_SSE2
		using group_type = __detail::group_sse2;
		using match_type = __detail::bitmask_sse2;
#else
		using group_type = __detail::group_fallback;
		using match_type = __detail::bitmask_fallback;
#endif

		using group_info = __detail::group_info<sizeof(group_type)>;
		static constexpr int group_size = static_cast<int>(sizeof(group_type));

		using slot = pair<K, T>;

	public:
		template<typename U>
		struct iterator_base {
			constexpr U& operator*() const {
				return slots[cursor];
			}

			constexpr U* operator->() const {
				return &slots[cursor];
			}

			constexpr bool operator==(iterator_base other) const {
				return cursor == other.cursor;
			}

			constexpr iterator_base& operator++() {
				// Consume the current match.
				group_matches.advance();
				seek();
				return *this;
			}

			constexpr iterator_base& operator--() {
				size_t i = cursor;

				while (i) {
					--i;
					if (!(ctrl[i] & 0x80)) {
						cursor = i;
						calculate_mask();
						return *this;
					}
				}

				return *this;
			}

		private:
			friend unordered_map;

			constexpr void seek() {
				while (!group_matches) {
					group_base += group_size;
					if (group_base >= cap) {
						cursor = cap;
						return;
					}

					auto group = group_type::load(ctrl, group_base);
					group_matches = group.match_full();
				}

				cursor = group_base + group_matches.lowest_byte();
			}

			constexpr void calculate_mask() {
				group_base = cursor & ~(group_size - 1);
				auto group = group_type::load(ctrl, group_base);
				group_matches = group.match_full();
				// Clear matches below cursor.
				group_matches.clear_below(static_cast<unsigned int>(cursor - group_base));
			}

			constexpr iterator_base(U* slots, uint8_t* ctrl, size_t cap, size_t cursor)
				: slots {slots}, ctrl {ctrl}, cap {cap}, cursor {cursor} {}

			U* slots;
			uint8_t* ctrl;
			size_t cap;
			size_t cursor;
			size_t group_base {};
			match_type group_matches {};
		};

		using iterator = iterator_base<slot>;
		using const_iterator = iterator_base<const slot>;

		constexpr unordered_map() = default;

		constexpr explicit unordered_map(Allocator alloc) : alloc {std::move(alloc)} {}

		constexpr unordered_map(unordered_map&& other) noexcept
			: bucket_mask {other.bucket_mask}, ctrl {other.ctrl}, slots {other.slots},
			size_ {other.size_}, capacity_ {other.capacity_}, growth_left {other.growth_left},
			alloc {std::move(other.alloc)} {

			other.bucket_mask = 0;
			other.ctrl = const_cast<uint8_t*>(group_info::empty);
			other.slots = nullptr;
			other.size_ = 0;
			other.capacity_ = 0;
			other.growth_left = 0;
		}

		unordered_map(const unordered_map& other) requires (!Allocator::infallible) = delete;
		constexpr unordered_map(const unordered_map& other) requires Allocator::infallible : alloc {other.alloc} {
			extend_from(other);
		}

		constexpr unordered_map& operator=(unordered_map&& other) noexcept {
			destroy();

			bucket_mask = other.bucket_mask;
			ctrl = other.ctrl;
			slots = other.slots;
			size_ = other.size_;
			capacity_ = other.capacity_;
			growth_left = other.growth_left;
			alloc = std::move(other.alloc);

			other.bucket_mask = 0;
			other.ctrl = const_cast<uint8_t*>(group_info::empty);
			other.slots = nullptr;
			other.size_ = 0;
			other.capacity_ = 0;
			other.growth_left = 0;

			return *this;
		}

		unordered_map& operator=(const unordered_map& other) requires (!Allocator::infallible) = delete;
		constexpr unordered_map& operator=(const unordered_map& other) requires Allocator::infallible {
			clear();
			extend_from(other);
			return *this;
		}

		// Extend the map with keys in `other`.
		// Note: Duplicate keys are ignored.
		constexpr alloc_res extend_from(const unordered_map& other) {
			if (growth_left < other.size_) {
				if (auto res = reserve(size_ + other.size_); !res) {
					return res;
				}
			}

			for (auto& entry : other) {
				if (contains(entry.first)) {
					continue;
				}

				auto hash = Hasher {}.hash(entry.first).finish();

				size_t index = find_insert_slot(hash, bucket_mask, ctrl, growth_left);

				if constexpr (!std::is_same_v<T, void>) {
					std::construct_at(&slots[index], pair<K, T>(entry.first, entry.second));
				}
				else {
					std::construct_at(&slots[index], pair<K, T>(entry.first));
				}

				++size_;
			}

			return alloc_res::success();
		}

		// Extend the map with keys in `other`.
		// Note: Duplicate keys are ignored.
		template<typename Key = K, typename Value = T>
		constexpr alloc_res extend_from(std::initializer_list<pair<Key, Value>> list) {
			if (growth_left < list.size()) {
				if (auto res = reserve(size_ + list.size()); !res) {
					return res;
				}
			}

			for (auto& entry : list) {
				if (contains(entry.first)) {
					continue;
				}

				auto hash = Hasher {}.hash(entry.first).finish();

				size_t index = find_insert_slot(hash, bucket_mask, ctrl, growth_left);

				if constexpr (!std::is_same_v<T, void>) {
					std::construct_at(&slots[index], pair<K, T>(entry.first, entry.second));
				}
				else {
					std::construct_at(&slots[index], pair<K, T>(entry.first));
				}

				++size_;
			}

			return alloc_res::success();
		}

		template<typename K2 = K>
		[[nodiscard]] constexpr iterator find(const K2& key) {
			auto hash = Hasher {}.hash(key).finish();

			uint8_t group_hash = hash & 0x7F;
			hash >>= 7;

			size_t bucket = static_cast<size_t>(hash) & bucket_mask;
			size_t stride = group_size;

			while (true) {
				auto group = group_type::load(ctrl, bucket);
				auto matches = group.match_byte(group_hash);

				while (matches) {
					int index = matches.lowest_byte();

					auto& slot = get_slot(bucket + index);
					if (slot.first == key) {
						iterator it {slots, ctrl, capacity_, (bucket + index) & bucket_mask};
						it.calculate_mask();
						return it;
					}

					matches.advance();
				}

				matches = group.match_empty();
				if (matches) {
					return end();
				}

				bucket = (bucket + stride) & bucket_mask;
				stride += group_size;
			}
		}

		template<typename K2 = K>
		[[nodiscard]] constexpr const_iterator find(const K2& key) const {
			auto hash = Hasher {}.hash(key).finish();

			uint8_t group_hash = hash & 0x7F;
			hash >>= 7;

			size_t bucket = hash & bucket_mask;
			size_t stride = group_size;

			while (true) {
				auto group = group_type::load(ctrl, bucket);
				auto matches = group.match_byte(group_hash);

				while (matches) {
					int index = matches.lowest_byte();

					auto& slot = get_slot(bucket + index);
					if (slot.first == key) {
						const_iterator it {slots, ctrl, capacity_, (bucket + index) & bucket_mask};
						it.calculate_mask();
						return it;
					}

					matches.advance();
				}

				matches = group.match_empty();
				if (matches) {
					return end();
				}

				bucket = (bucket + stride) & bucket_mask;
				stride += group_size;
			}
		}

		template<typename Key = K>
		[[nodiscard]] constexpr bool contains(const Key& key) const {
			return find(key) != end();
		}

		// Inserts a given (key, value) pair to the map.
		// Returns true if inserted, false otherwise.
		template<typename Key = K, typename Value = T>
		constexpr insert_alloc_res insert(Key&& key, Value&& value) {
			auto hash = Hasher {}.hash(key).finish();

			size_t index = find_insert_slot_or_get_no_mark(hash, key);
			uint8_t byte = ctrl[index];

			if (byte == __detail::EMPTY) {
				if (growth_left == 0) {
					if (auto res = reserve_one(); !res) {
						return insert_alloc_res::fail();
					}

					index = find_insert_slot_no_mark(hash, bucket_mask, ctrl);
				}

				--growth_left;
			}
			else if (byte != __detail::TOMBSTONE) {
				return insert_alloc_res::success(false);
			}

			uint8_t group_hash = hash & 0x7F;
			set_ctrl_byte(ctrl, bucket_mask, index, group_hash);

			std::construct_at(&slots[index], pair<K, T>(std::forward<Key>(key), std::forward<Value>(value)));
			++size_;

			return insert_alloc_res::success(true);
		}

		// Inserts a given key to the map.
		// Returns true if inserted, false otherwise.
		template<typename Key = K> requires std::is_same_v<T, void>
		constexpr insert_alloc_res insert(Key&& key) {
			auto hash = Hasher {}.hash(key).finish();

			size_t index = find_insert_slot_or_get_no_mark(hash, key);
			uint8_t byte = ctrl[index];

			if (byte == __detail::EMPTY) {
				if (growth_left == 0) {
					if (auto res = reserve_one(); !res) {
						return insert_alloc_res::fail();
					}

					index = find_insert_slot_no_mark(hash, bucket_mask, ctrl);
				}

				--growth_left;
			}
			else if (byte != __detail::TOMBSTONE) {
				return insert_alloc_res::success(false);
			}

			uint8_t group_hash = hash & 0x7F;
			set_ctrl_byte(ctrl, bucket_mask, index, group_hash);

			std::construct_at(&slots[index], pair<K, T>(std::forward<Key>(key)));
			++size_;

			return insert_alloc_res::success(true);
		}

		// Inserts a given (key, value) pair to the map
		// replacing any old entry with the same key.
		// Returns true if a new key was inserted, false otherwise.
		template<typename Key = K, typename Value = T>
		constexpr insert_alloc_res insert_or_overwrite(Key&& key, Value&& value) {
			auto hash = Hasher {}.hash(key).finish();

			size_t index = find_insert_slot_or_get_no_mark(hash, key);
			uint8_t byte = ctrl[index];

			if (byte == __detail::EMPTY) {
				if (growth_left == 0) {
					if (auto res = reserve_one(); !res) {
						return insert_alloc_res::fail();
					}

					index = find_insert_slot_no_mark(hash, bucket_mask, ctrl);
				}

				--growth_left;
			}
			else if (byte != __detail::TOMBSTONE) {
				slots[index].first = std::forward<Key>(key);
				slots[index].second = std::forward<Value>(value);
				return insert_alloc_res::success(false);
			}

			uint8_t group_hash = hash & 0x7F;
			set_ctrl_byte(ctrl, bucket_mask, index, group_hash);

			std::construct_at(&slots[index], pair<K, T>(std::forward<Key>(key), std::forward<Value>(value)));
			++size_;

			return insert_alloc_res::success(true);
		}

		// Inserts a given key to the map
		// replacing any old entry with the same key.
		// Returns true if a new key was inserted, false otherwise.
		template<typename Key = K> requires std::is_same_v<T, void>
		constexpr insert_alloc_res insert_or_overwrite(Key&& key) {
			auto hash = Hasher {}.hash(key).finish();

			size_t index = find_insert_slot_or_get_no_mark(hash, key);
			uint8_t byte = ctrl[index];

			if (byte == __detail::EMPTY) {
				if (growth_left == 0) {
					if (auto res = reserve_one(); !res) {
						return insert_alloc_res::fail();
					}

					index = find_insert_slot_no_mark(hash, bucket_mask, ctrl);
				}

				--growth_left;
			}
			else if (byte != __detail::TOMBSTONE) {
				slots[index].first = std::forward<Key>(key);
				return insert_alloc_res::success(false);
			}

			uint8_t group_hash = hash & 0x7F;
			set_ctrl_byte(ctrl, bucket_mask, index, group_hash);

			std::construct_at(&slots[index], pair<K, T>(std::forward<Key>(key)));
			++size_;

			return insert_alloc_res::success(true);
		}

		// Inserts a given (key, value) pair to the map.
		// Note: This function does not guard against duplicates.
		template<typename Key = K, typename Value = T>
		constexpr alloc_res insert_unchecked(Key&& key, Value&& value) {
			if (auto res = reserve_one(); !res) {
				return alloc_res::fail();
			}

			auto hash = Hasher {}.hash(key).finish();

			size_t index = find_insert_slot(hash, bucket_mask, ctrl, growth_left);
			std::construct_at(&slots[index], pair<K, T>(std::forward<Key>(key), std::forward<Value>(value)));
			++size_;

			return alloc_res::success();
		}

		// Inserts a given key to the map.
		// Note: This function does not guard against duplicates.
		template<typename Key = K> requires std::is_same_v<T, void>
		constexpr alloc_res insert_unchecked(Key&& key) {
			if (auto res = reserve_one(); !res) {
				return alloc_res::fail();
			}

			auto hash = Hasher {}.hash(key).finish();

			size_t index = find_insert_slot(hash, bucket_mask, ctrl, growth_left);
			std::construct_at(&slots[index], pair<K, T>(std::forward<Key>(key)));
			++size_;

			return alloc_res::success();
		}

		constexpr alloc_res reserve(size_t new_cap) {
			if (new_cap <= size_ + growth_left) {
				return alloc_res::success();
			}

			size_t max_growth = get_capacity_until_resize(bucket_mask);
			if (new_cap <= max_growth / 2) {
				rehash();
				return alloc_res::success();
			}

			if (new_cap < max_growth + 1) {
				new_cap = max_growth + 1;
			}

			size_t actual_new_cap = get_new_capacity(new_cap);
			return reallocate(actual_new_cap);
		}

		constexpr alloc_res reserve_one() {
			if (growth_left != 0) {
				return alloc_res::success();
			}

			size_t max_growth = get_capacity_until_resize(bucket_mask);

			if (size_ < max_growth - max_growth / 4) {
				rehash();
				return alloc_res::success();
			}
			else {
				return reallocate(get_new_capacity(max_growth + 1));
			}
		}

		constexpr void erase(iterator it) {
			size_t bucket = it.cursor & bucket_mask;

			std::destroy_at(&slots[bucket]);

			size_t prev_group_bucket = (bucket - group_size) & bucket_mask;

			auto group = group_type::load(ctrl, bucket);
			auto matches = group.match_empty();

			auto prev_group = group_type::load(ctrl, prev_group_bucket);
			auto prev_matches = prev_group.match_empty();

			if (matches.lowest_byte() + prev_matches.highest_byte() >= group_size) {
				set_ctrl_byte(ctrl, bucket_mask, bucket, __detail::TOMBSTONE);
			}
			else {
				set_ctrl_byte(ctrl, bucket_mask, bucket, __detail::EMPTY);
				++growth_left;
			}

			--size_;
		}

		template<typename Key = K>
		constexpr bool erase(const Key& key) {
			auto it = find(key);

			if (it != end()) {
				erase(it);
				return true;
			}

			return false;
		}

		constexpr void clear() {
			if (!slots) {
				return;
			}

			for (size_t i = 0; i < capacity_; i += group_size) {
				auto group = group_type::load(ctrl, i);
				auto matches = group.match_full();

				while (matches) {
					int index = matches.lowest_byte();
					std::destroy_at(&get_slot(i + index));
					matches.advance();
				}
			}

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < capacity_ + group_size; ++i) {
					ctrl[i] = __detail::EMPTY;
				}
			}
			else {
				memory_set(ctrl, __detail::EMPTY, capacity_ + group_size);
			}

			size_ = 0;
			growth_left = get_capacity_until_resize(bucket_mask);
		}

		[[nodiscard]] constexpr size_t size() const {
			return size_;
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return !size_;
		}

		[[nodiscard]] constexpr iterator begin() {
			iterator it {slots, ctrl, capacity_, 0};
			it.calculate_mask();
			it.seek();
			return it;
		}

		[[nodiscard]] constexpr const_iterator begin() const {
			const_iterator it {slots, ctrl, capacity_, 0};
			it.calculate_mask();
			it.seek();
			return it;
		}

		[[nodiscard]] constexpr iterator end() {
			return iterator {slots, ctrl, capacity_, capacity_};
		}

		[[nodiscard]] constexpr const_iterator end() const {
			return const_iterator {slots, ctrl, capacity_, capacity_};
		}

		constexpr ~unordered_map() {
			destroy();
		}

	private:
		constexpr void destroy() {
			clear();

			if (slots) {
				if (std::is_constant_evaluated()) {
					stl_allocator<uint8_t>::deallocate(ctrl, capacity_ + group_size);
					alloc.deallocate(slots, capacity_);
				}
				else {
					alloc.deallocate_bytes(
						reinterpret_cast<unsigned char*>(slots),
						capacity_ * sizeof(slot) + capacity_ + group_size);
				}
			}
		}

		static constexpr size_t find_insert_slot_no_mark(
			typename Hasher::hash_type hash,
			size_t bucket_mask,
			uint8_t* ctrl) {

			hash >>= 7;

			size_t bucket = static_cast<size_t>(hash) & bucket_mask;
			size_t stride = group_size;

			while (true) {
				auto group = group_type::load(ctrl, bucket);
				auto matches = group.match_empty_or_tombstone();

				if (matches) {
					bucket += matches.lowest_byte();
					bucket &= bucket_mask;

					// If the table is less than a group in size then the index can land on an EMPTY padding byte
					// before the mirror (and point to a FULL bucket).
					// a b c d EMPTY... mirror(a) mirror(b) mirror(c) mirror(d)
					if (bucket_mask < group_size - 1 && !(ctrl[bucket] & 0x80)) {
						// Check the whole group 0.
						group = group_type::load(ctrl, 0);
						matches = group.match_empty_or_tombstone();

						bucket = matches.lowest_byte();
						bucket &= bucket_mask;
					}

					return bucket;
				}

				bucket = (bucket + stride) & bucket_mask;
				stride += group_size;
			}
		}

		template<typename Key>
		constexpr size_t find_insert_slot_or_get_no_mark(
			typename Hasher::hash_type hash,
			const Key& key) {

			uint8_t group_hash = hash & 0x7F;
			hash >>= 7;

			size_t bucket = static_cast<size_t>(hash) & bucket_mask;
			size_t stride = group_size;

			auto first_insert_slot = size_t(-1);

			while (true) {
				auto group = group_type::load(ctrl, bucket);

				auto search_matches = group.match_byte(group_hash);
				while (search_matches) {
					int index = search_matches.lowest_byte();

					auto& slot = slots[(bucket + index) & bucket_mask];
					if (slot.first == key) {
						return (bucket + index) & bucket_mask;
					}

					search_matches.advance();
				}

				if (first_insert_slot == size_t(-1)) {
					auto matches = group.match_empty_or_tombstone();

					if (matches) {
						first_insert_slot = bucket + matches.lowest_byte();
						first_insert_slot &= bucket_mask;

						// If the table is less than a group in size then the index can land on an EMPTY padding byte
						// before the mirror (and point to a FULL bucket).
						// a b c d EMPTY... mirror(a) mirror(b) mirror(c) mirror(d)
						if (bucket_mask < group_size - 1 && !(ctrl[first_insert_slot] & 0x80)) {
							// Check the whole group 0.
							group = group_type::load(ctrl, 0);
							matches = group.match_empty_or_tombstone();

							first_insert_slot = matches.lowest_byte();
							first_insert_slot &= bucket_mask;
						}
					}
				}

				if (group.match_empty()) {
					break;
				}

				bucket = (bucket + stride) & bucket_mask;
				stride += group_size;
			}

			return first_insert_slot;
		}

		static constexpr size_t find_insert_slot(
			typename Hasher::hash_type hash,
			size_t bucket_mask,
			uint8_t* ctrl,
			size_t& growth_left) {

			uint8_t group_hash = hash & 0x7F;

			size_t bucket = find_insert_slot_no_mark(hash, bucket_mask, ctrl);

			if (ctrl[bucket] == __detail::EMPTY) {
				--growth_left;
			}

			set_ctrl_byte(ctrl, bucket_mask, bucket, group_hash);
			return bucket;
		}

		[[nodiscard]] static constexpr size_t get_capacity_until_resize(size_t bucket_mask) {
			// 87.5% load factor if more than a group in size.
			return bucket_mask < static_cast<size_t>(group_size) ? bucket_mask : ((bucket_mask + 1) / 8 * 7);
		}

		[[nodiscard]] static constexpr size_t get_new_capacity(size_t new_size) {
			if (new_size < 4) {
				return 4;
			}
			if (new_size < 8) {
				return 8;
			}

			return bit_ceil(new_size * 8 / 7);
		}

		constexpr alloc_res reallocate(size_t new_cap) {
			uint8_t* new_ctrl;
			slot* new_slots;

			if (std::is_constant_evaluated()) {
				new_ctrl = stl_allocator<uint8_t>::allocate(new_cap + group_size);
				if (!new_ctrl) {
					return alloc_res::fail();
				}

				for (size_t i = 0; i < new_cap + group_size; ++i) {
					std::construct_at(&new_ctrl[i], __detail::EMPTY);
				}

				new_slots = alloc.allocate(new_cap);
				if (!new_slots) {
					return alloc_res::fail();
				}
			}
			else {
				unsigned char* bytes = alloc.allocate_bytes(new_cap * sizeof(slot) + new_cap + group_size);
				if (!bytes) {
					return alloc_res::fail();
				}

				new_slots = reinterpret_cast<slot*>(bytes);
				new_ctrl = reinterpret_cast<uint8_t*>(bytes + new_cap * sizeof(slot));

				memory_set(new_ctrl, __detail::EMPTY, new_cap + group_size);
			}

			size_t new_bucket_mask = new_cap - 1;

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < capacity_; ++i) {
					if (!(ctrl[i] & 0x80)) {
						slot& slot = slots[i];

						auto hash = Hasher {}.hash(slot.first).finish();

						size_t new_index = find_insert_slot(hash, new_bucket_mask, new_ctrl, growth_left);
						std::construct_at(&new_slots[new_index], std::move(slot));

						std::destroy_at(&slot);
					}
				}
			}
			else {
				for (size_t i = 0; i < capacity_; i += group_size) {
					auto group = group_type::load(ctrl, i);
					auto matches = group.match_full();

					while (matches) {
						int index = matches.lowest_byte();

						slot& slot = slots[i + index];

						auto hash = Hasher {}.hash(slot.first).finish();

						size_t new_index = find_insert_slot(hash, new_bucket_mask, new_ctrl, growth_left);
						std::construct_at(&new_slots[new_index], std::move(slot));

						std::destroy_at(&slot);
						matches.advance();
					}
				}
			}

			if (slots) {
				if (std::is_constant_evaluated()) {
					stl_allocator<uint8_t>::deallocate(ctrl, capacity_ + group_size);
					alloc.deallocate(slots, capacity_);
				}
				else {
					alloc.deallocate_bytes(
						reinterpret_cast<unsigned char*>(slots),
						capacity_ * sizeof(slot) + capacity_ + group_size);
				}
			}

			bucket_mask = new_bucket_mask;
			ctrl = new_ctrl;
			slots = new_slots;
			capacity_ = new_cap;
			growth_left = get_capacity_until_resize(bucket_mask) - size_;

			return alloc_res::success();
		}

		constexpr void rehash() {
			using std::swap;

			for (size_t i = 0; i < capacity_; ++i) {
				uint8_t byte = ctrl[i];

				if (byte == __detail::TOMBSTONE) {
					ctrl[i] = __detail::EMPTY;
				}
				else if (!(byte & 0x80)) {
					ctrl[i] = __detail::TOMBSTONE;
				}
			}

			// Update the mirrored copies.
			for (size_t i = 0; i < capacity_ && i < static_cast<size_t>(group_size); ++i) {
				set_ctrl_byte(ctrl, bucket_mask, i, ctrl[i]);
			}

			auto get_group_index = [&](size_t start_bucket, size_t i) {
				return ((i - start_bucket) & bucket_mask) / group_size;
			};

			for (size_t i = 0; i < capacity_; ++i) {
				uint8_t byte = ctrl[i];
				if (byte != __detail::TOMBSTONE) {
					continue;
				}

				slot& s = slots[i];

				auto hash = Hasher {}.hash(s.first).finish();
				uint8_t group_hash = hash & 0x7F;

				size_t start_bucket = (hash >> 7) & bucket_mask;
				size_t new_bucket = find_insert_slot_no_mark(hash, bucket_mask, ctrl);

				if (get_group_index(start_bucket, i) == get_group_index(start_bucket, new_bucket)) {
					set_ctrl_byte(ctrl, bucket_mask, i, group_hash);
					continue;
				}

				if (ctrl[new_bucket] == __detail::EMPTY) {
					set_ctrl_byte(ctrl, bucket_mask, new_bucket, group_hash);
					set_ctrl_byte(ctrl, bucket_mask, i, __detail::EMPTY);

					std::construct_at(&slots[new_bucket], std::move(s));
					std::destroy_at(&s);
				}
				else {
					set_ctrl_byte(ctrl, bucket_mask, new_bucket, group_hash);
					swap(s, slots[new_bucket]);
					--i;
				}
			}

			growth_left = get_capacity_until_resize(bucket_mask) - size_;
		}

		static constexpr void set_ctrl_byte(uint8_t* ctrl, uint64_t bucket_mask, size_t index, uint8_t byte) {
			ctrl[index] = byte;
			// Mirror the first group_size - 1 bytes at the end.
			ctrl[((index - group_size) & bucket_mask) + group_size] = byte;
		}

		constexpr slot& get_slot(size_t index) {
			return slots[index & bucket_mask];
		}

		constexpr const slot& get_slot(size_t index) const {
			return slots[index & bucket_mask];
		}

		size_t bucket_mask {};
		uint8_t* ctrl {const_cast<uint8_t*>(group_info::empty)};
		slot* slots {};
		size_t size_ {};
		size_t capacity_ {};
		size_t growth_left {};
#if defined(_MSC_VER) && !defined(__clang__)
		[[msvc::no_unique_address]]
#else
		[[no_unique_address]]
#endif
		Allocator alloc {};
	};

	template<typename K, allocator<pair<K, void>> Allocator = stl_allocator<pair<K, void>>, hasher Hasher = fx_hasher>
	using unordered_set = unordered_map<K, void, Allocator, Hasher>;
}
