#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bit.hpp"

namespace hz {
	template<int N>
	class bitset {
		static constexpr int slice_bits = static_cast<int>(sizeof(uintptr_t)) * 8;
		static constexpr int num_slices = (N + slice_bits - 1) / slice_bits;

	public:
		static constexpr int bits = N;

		constexpr bitset() = default;

		constexpr void set(int bit) {
			value_[bit / slice_bits] |= uintptr_t {1} << (bit % slice_bits);
		}

		constexpr void set_range(int start, int count) {
			int i = start / slice_bits;
			int offset = start % slice_bits;

			if (offset != 0) {
				int to_set_first = slice_bits - offset;
				if (count < to_set_first) {
					to_set_first = count;
				}

				value_[i++] |= mask_for_bits(to_set_first) << offset;
				count -= to_set_first;

				if (!count) {
					return;
				}
			}

			while (count >= slice_bits) {
				value_[i++] = ~uintptr_t {0};
				count -= slice_bits;
			}

			if (count % slice_bits) {
				value_[i] |= mask_for_bits(count % slice_bits);
			}
		}

		constexpr void clear(int bit) {
			value_[bit / slice_bits] &= ~(uintptr_t {1} << (bit % slice_bits));
		}

		constexpr void clear_range(int start, int count) {
			int i = start / slice_bits;
			int offset = start % slice_bits;

			if (offset != 0) {
				int to_set_first = slice_bits - offset;
				if (count < to_set_first) {
					to_set_first = count;
				}

				value_[i++] &= ~(mask_for_bits(to_set_first) << offset);
				count -= to_set_first;

				if (!count) {
					return;
				}
			}

			while (count >= slice_bits) {
				value_[i++] = 0;
				count -= slice_bits;
			}

			if (count % slice_bits) {
				value_[i] &= ~mask_for_bits(count % slice_bits);
			}
		}

		[[nodiscard]] constexpr bool get(int bit) const {
			return (value_[bit / slice_bits] >> (bit % slice_bits)) & 1;
		}

		// Finds the first set bit starting at `start`.
		// Returns `bit + 1` if a set bit was found, 0 otherwise.
		[[nodiscard]] constexpr int find_first_set(int start = 0) const {
			int i = start / slice_bits;
			int offset = start % slice_bits;

			uintptr_t val = value_[i] & (~uintptr_t {0} << offset);
			if (val != 0) {
				return i * slice_bits + countr_zero(val) + 1;
			}

			++i;

			for (; i < num_slices; ++i) {
				val = value_[i];
				if (val != 0) {
					return i * slice_bits + countr_zero(val) + 1;
				}
			}

			return 0;
		}

		// Finds the first clean bit starting at `start`.
		// Returns `bit + 1` if a clean bit was found, 0 otherwise.
		[[nodiscard]] constexpr int find_first_clean(int start = 0) const {
			int i = start / slice_bits;
			int offset = start % slice_bits;

			uintptr_t val = value_[i] | mask_for_bits(offset);
			if (val != ~uintptr_t {0}) {
				int bit = i * slice_bits + countr_zero(~val);
				return bit < N ? bit + 1 : 0;
			}

			++i;

			for (; i < num_slices; ++i) {
				val = value_[i];
				if (val != ~uintptr_t {0}) {
					int bit = i * slice_bits + countr_zero(~val);
					return bit < N ? bit + 1 : 0;
				}
			}

			return 0;
		}

		[[nodiscard]] constexpr int count() const {
			int count = 0;

			for (auto val : value_) {
				count += popcount(val);
			}

			return count;
		}

	private:
		static constexpr uintptr_t mask_for_bits(int count) {
			return (uintptr_t {1} << count) - 1;
		}

		uintptr_t value_[num_slices] {};
	};
}
