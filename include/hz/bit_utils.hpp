#pragma once

#include <stdint.h>

namespace hz {
	// Returns 0x80 broadcast to every matching byte in `value`.
	constexpr uint64_t match_bytes(uint64_t value, uint8_t find) {
		constexpr uint64_t ones = 0x0101010101010101ULL;
		constexpr uint64_t half = 0x7F7F7F7F7F7F7F7FULL;

		uint64_t mask = find * ones;

		// Matched bytes are zeros.
		uint64_t step0 = value ^ mask;
		// Force any bytes <= 0x7F to have the high bit set.
		uint64_t step1 = (step0 & half) + half;

		// Merge in the high bits from step0 that can't be matches, fill the lower half with ones and negate.
		return ~(step1 | step0 | half);
	}

	// Returns 0x80 broadcast to every matching byte in `value`.
	constexpr uint32_t match_bytes(uint32_t value, uint8_t find) {
		constexpr uint32_t ones = 0x01010101U;
		constexpr uint32_t half = 0x7F7F7F7FU;

		uint32_t mask = find * ones;

		// Matched bytes are zeros.
		uint32_t step0 = value ^ mask;
		// Force any bytes <= 0x7F to have the high bit set.
		uint32_t step1 = (step0 & half) + half;

		// Merge in the high bits from step0 that can't be matches, fill the lower half with ones and negate.
		return ~(step1 | step0 | half);
	}
}
