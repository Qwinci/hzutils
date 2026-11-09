#include "alias_helper.hpp"

#include <stddef.h>
#include <stdint.h>

#ifndef HZ_MEM_SCALAR_PREFIX
#define HZ_MEM_SCALAR_PREFIX

#if defined(_MSC_VER) && !defined(__clang__)

extern "C" void* memset(void* dest, int ch, size_t size) noexcept;
extern "C" void* memcpy(void* __restrict dest, const void* __restrict src, size_t size) noexcept;
extern "C" void* memmove(void* dest, const void* src, size_t size) noexcept;

#pragma function(memset)
#pragma function(memcpy)
#pragma function(memmove)

#endif

#endif

#define CONCAT_HELPER(a, b) a ## b
#define CONCAT(a, b) CONCAT_HELPER(a, b)

#define HZ_MEM_FUNC(name) CONCAT(HZ_MEM_SCALAR_PREFIX, name)

extern "C" void* HZ_MEM_FUNC(memset)(void* dest, int ch, size_t size) noexcept {
	if (!size) {
		return dest;
	}

	auto* dest_ptr = static_cast<unsigned char*>(dest);

	auto mask = 0x0101010101010101ULL * static_cast<unsigned char>(ch);

	if (size < 4) {
		if (size & 2) {
			hz::aliased_store<uint16_t>(dest_ptr, mask);
			dest_ptr += 2;
		}

		if (size & 1) {
			*dest_ptr = mask;
		}

		return dest;
	}

	if (size <= 8) {
		hz::aliased_store<uint32_t>(dest_ptr, mask);
		hz::aliased_store<uint32_t>(dest_ptr + size - 4, mask);
		return dest;
	}

	if (size <= 16) {
		hz::aliased_store<uint64_t>(dest_ptr, mask);
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, mask);
		return dest;
	}

	if (size <= 32) {
		hz::aliased_store<uint64_t>(dest_ptr, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 8, mask);
		hz::aliased_store<uint64_t>(dest_ptr + size - 16, mask);
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, mask);
		return dest;
	}

	uintptr_t misalign = reinterpret_cast<uintptr_t>(dest) & 0x7;
	if (misalign) {
		hz::aliased_store<uint64_t>(dest_ptr, mask);
		dest_ptr += 8 - misalign;
		size -= 8 - misalign;

		if (size <= 32) {
			hz::aliased_store<uint64_t>(dest_ptr, mask);
			hz::aliased_store<uint64_t>(dest_ptr + 8, mask);
			hz::aliased_store<uint64_t>(dest_ptr + size - 16, mask);
			hz::aliased_store<uint64_t>(dest_ptr + size - 8, mask);
			return dest;
		}
	}

	if (size <= 64) {
		hz::aliased_store<uint64_t>(dest_ptr, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 8, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 16, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 24, mask);
		hz::aliased_store<uint64_t>(dest_ptr + size - 32, mask);
		hz::aliased_store<uint64_t>(dest_ptr + size - 24, mask);
		hz::aliased_store<uint64_t>(dest_ptr + size - 16, mask);
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, mask);
		return dest;
	}

	while (size > 64) {
		hz::aliased_store<uint64_t>(dest_ptr, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 8, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 16, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 24, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 32, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 40, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 48, mask);
		hz::aliased_store<uint64_t>(dest_ptr + 56, mask);

		dest_ptr += 64;
		size -= 64;
	}

	hz::aliased_store<uint64_t>(dest_ptr + size - 64, mask);
	hz::aliased_store<uint64_t>(dest_ptr + size - 56, mask);
	hz::aliased_store<uint64_t>(dest_ptr + size - 48, mask);
	hz::aliased_store<uint64_t>(dest_ptr + size - 40, mask);
	hz::aliased_store<uint64_t>(dest_ptr + size - 32, mask);
	hz::aliased_store<uint64_t>(dest_ptr + size - 24, mask);
	hz::aliased_store<uint64_t>(dest_ptr + size - 16, mask);
	hz::aliased_store<uint64_t>(dest_ptr + size - 8, mask);

	return dest;
}

extern "C" void* HZ_MEM_FUNC(memcpy)(void* __restrict dest, const void* __restrict src, size_t size) noexcept {
	if (!size) {
		return dest;
	}

	auto* dest_ptr = static_cast<unsigned char*>(dest);
	auto* src_ptr = static_cast<const unsigned char*>(src);

	if (size < 4) {
		if (size & 2) {
			hz::aliased_store<uint16_t>(dest_ptr, hz::aliased_load<uint16_t>(src_ptr));
			dest_ptr += 2;
			src_ptr += 2;
		}

		if (size & 1) {
			*dest_ptr = *src_ptr;
		}

		return dest;
	}

	if (size <= 8) {
		hz::aliased_store<uint32_t>(dest_ptr, hz::aliased_load<uint32_t>(src_ptr));
		hz::aliased_store<uint32_t>(dest_ptr + size - 4, hz::aliased_load<uint32_t>(src_ptr + size - 4));
		return dest;
	}

	if (size <= 16) {
		hz::aliased_store<uint64_t>(dest_ptr, hz::aliased_load<uint64_t>(src_ptr));
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, hz::aliased_load<uint64_t>(src_ptr + size - 8));
		return dest;
	}

	if (size <= 32) {
		auto first = hz::aliased_load<uint64_t>(src_ptr);
		auto second = hz::aliased_load<uint64_t>(src_ptr + 8);
		auto third = hz::aliased_load<uint64_t>(src_ptr + size - 16);
		auto fourth = hz::aliased_load<uint64_t>(src_ptr + size - 8);

		hz::aliased_store<uint64_t>(dest_ptr, first);
		hz::aliased_store<uint64_t>(dest_ptr + 8, second);
		hz::aliased_store<uint64_t>(dest_ptr + size - 16, third);
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, fourth);

		return dest;
	}

	uintptr_t misalign = reinterpret_cast<uintptr_t>(dest) & 0x7;
	if (misalign) {
		hz::aliased_store<uint64_t>(dest_ptr, hz::aliased_load<uint64_t>(src_ptr));

		dest_ptr += 8 - misalign;
		src_ptr += 8 - misalign;
		size -= 8 - misalign;

		if (size <= 32) {
			auto first = hz::aliased_load<uint64_t>(src_ptr);
			auto second = hz::aliased_load<uint64_t>(src_ptr + 8);
			auto third = hz::aliased_load<uint64_t>(src_ptr + size - 16);
			auto fourth = hz::aliased_load<uint64_t>(src_ptr + size - 8);

			hz::aliased_store<uint64_t>(dest_ptr, first);
			hz::aliased_store<uint64_t>(dest_ptr + 8, second);
			hz::aliased_store<uint64_t>(dest_ptr + size - 16, third);
			hz::aliased_store<uint64_t>(dest_ptr + size - 8, fourth);

			return dest;
		}
	}

	if (size <= 64) {
		auto m0 = hz::aliased_load<uint64_t>(src_ptr);
		auto m1 = hz::aliased_load<uint64_t>(src_ptr + 8);
		auto m2 = hz::aliased_load<uint64_t>(src_ptr + 16);
		auto m3 = hz::aliased_load<uint64_t>(src_ptr + 24);
		auto m4 = hz::aliased_load<uint64_t>(src_ptr + size - 32);
		auto m5 = hz::aliased_load<uint64_t>(src_ptr + size - 24);
		auto m6 = hz::aliased_load<uint64_t>(src_ptr + size - 16);
		auto m7 = hz::aliased_load<uint64_t>(src_ptr + size - 8);

		hz::aliased_store<uint64_t>(dest_ptr, m0);
		hz::aliased_store<uint64_t>(dest_ptr + 8, m1);
		hz::aliased_store<uint64_t>(dest_ptr + 16, m2);
		hz::aliased_store<uint64_t>(dest_ptr + 24, m3);
		hz::aliased_store<uint64_t>(dest_ptr + size - 32, m4);
		hz::aliased_store<uint64_t>(dest_ptr + size - 24, m5);
		hz::aliased_store<uint64_t>(dest_ptr + size - 16, m6);
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, m7);

		return dest;
	}

	while (size > 64) {
		auto m0 = hz::aliased_load<uint64_t>(src_ptr);
		auto m1 = hz::aliased_load<uint64_t>(src_ptr + 8);
		auto m2 = hz::aliased_load<uint64_t>(src_ptr + 16);
		auto m3 = hz::aliased_load<uint64_t>(src_ptr + 24);
		auto m4 = hz::aliased_load<uint64_t>(src_ptr + 32);
		auto m5 = hz::aliased_load<uint64_t>(src_ptr + 40);
		auto m6 = hz::aliased_load<uint64_t>(src_ptr + 48);
		auto m7 = hz::aliased_load<uint64_t>(src_ptr + 56);

		hz::aliased_store<uint64_t>(dest_ptr, m0);
		hz::aliased_store<uint64_t>(dest_ptr + 8, m1);
		hz::aliased_store<uint64_t>(dest_ptr + 16, m2);
		hz::aliased_store<uint64_t>(dest_ptr + 24, m3);
		hz::aliased_store<uint64_t>(dest_ptr + 32, m4);
		hz::aliased_store<uint64_t>(dest_ptr + 40, m5);
		hz::aliased_store<uint64_t>(dest_ptr + 48, m6);
		hz::aliased_store<uint64_t>(dest_ptr + 56, m7);

		dest_ptr += 64;
		src_ptr += 64;
		size -= 64;
	}

	auto m0 = hz::aliased_load<uint64_t>(src_ptr + size - 64);
	auto m1 = hz::aliased_load<uint64_t>(src_ptr + size - 56);
	auto m2 = hz::aliased_load<uint64_t>(src_ptr + size - 48);
	auto m3 = hz::aliased_load<uint64_t>(src_ptr + size - 40);
	auto m4 = hz::aliased_load<uint64_t>(src_ptr + size - 32);
	auto m5 = hz::aliased_load<uint64_t>(src_ptr + size - 24);
	auto m6 = hz::aliased_load<uint64_t>(src_ptr + size - 16);
	auto m7 = hz::aliased_load<uint64_t>(src_ptr + size - 8);

	hz::aliased_store<uint64_t>(dest_ptr + size - 64, m0);
	hz::aliased_store<uint64_t>(dest_ptr + size - 56, m1);
	hz::aliased_store<uint64_t>(dest_ptr + size - 48, m2);
	hz::aliased_store<uint64_t>(dest_ptr + size - 40, m3);
	hz::aliased_store<uint64_t>(dest_ptr + size - 32, m4);
	hz::aliased_store<uint64_t>(dest_ptr + size - 24, m5);
	hz::aliased_store<uint64_t>(dest_ptr + size - 16, m6);
	hz::aliased_store<uint64_t>(dest_ptr + size - 8, m7);

	return dest;
}

extern "C" void* HZ_MEM_FUNC(memmove)(void* dest, const void* src, size_t size) noexcept {
	if (!size) {
		return dest;
	}

	auto* dest_ptr = static_cast<unsigned char*>(dest);
	auto* src_ptr = static_cast<const unsigned char*>(src);

	if (size < 4) {
		if (size & 2) {
			if (size & 1) {
				auto first = hz::aliased_load<uint16_t>(src_ptr);
				uint8_t second = src_ptr[2];
				hz::aliased_store<uint16_t>(dest_ptr, first);
				dest_ptr[2] = second;
			}
			else {
				hz::aliased_store<uint16_t>(dest_ptr, hz::aliased_load<uint16_t>(src_ptr));
			}
		}
		else if (size & 1) {
			*dest_ptr = *src_ptr;
		}

		return dest;
	}

	if (size <= 8) {
		auto first = hz::aliased_load<uint32_t>(src_ptr);
		auto second = hz::aliased_load<uint32_t>(src_ptr + size - 4);

		hz::aliased_store<uint32_t>(dest_ptr, first);
		hz::aliased_store<uint32_t>(dest_ptr + size - 4, second);
		return dest;
	}

	if (size <= 16) {
		auto first = hz::aliased_load<uint64_t>(src_ptr);
		auto second = hz::aliased_load<uint64_t>(src_ptr + size - 8);

		hz::aliased_store<uint64_t>(dest_ptr, first);
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, second);
		return dest;
	}

	if (size <= 32) {
		auto first = hz::aliased_load<uint64_t>(src_ptr);
		auto second = hz::aliased_load<uint64_t>(src_ptr + 8);
		auto third = hz::aliased_load<uint64_t>(src_ptr + size - 16);
		auto fourth = hz::aliased_load<uint64_t>(src_ptr + size - 8);

		hz::aliased_store<uint64_t>(dest_ptr, first);
		hz::aliased_store<uint64_t>(dest_ptr + 8, second);
		hz::aliased_store<uint64_t>(dest_ptr + size - 16, third);
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, fourth);

		return dest;
	}

	if (size <= 64) {
		auto m0 = hz::aliased_load<uint64_t>(src_ptr);
		auto m1 = hz::aliased_load<uint64_t>(src_ptr + 8);
		auto m2 = hz::aliased_load<uint64_t>(src_ptr + 16);
		auto m3 = hz::aliased_load<uint64_t>(src_ptr + 24);
		auto m4 = hz::aliased_load<uint64_t>(src_ptr + size - 32);
		auto m5 = hz::aliased_load<uint64_t>(src_ptr + size - 24);
		auto m6 = hz::aliased_load<uint64_t>(src_ptr + size - 16);
		auto m7 = hz::aliased_load<uint64_t>(src_ptr + size - 8);

		hz::aliased_store<uint64_t>(dest_ptr, m0);
		hz::aliased_store<uint64_t>(dest_ptr + 8, m1);
		hz::aliased_store<uint64_t>(dest_ptr + 16, m2);
		hz::aliased_store<uint64_t>(dest_ptr + 24, m3);
		hz::aliased_store<uint64_t>(dest_ptr + size - 32, m4);
		hz::aliased_store<uint64_t>(dest_ptr + size - 24, m5);
		hz::aliased_store<uint64_t>(dest_ptr + size - 16, m6);
		hz::aliased_store<uint64_t>(dest_ptr + size - 8, m7);

		return dest;
	}

	size_t orig_size = size;

	auto* dest_end = dest_ptr + size;
	auto* src_end = src_ptr + size;

	auto tail0 = hz::aliased_load<uint64_t>(src_end - 64);
	auto tail1 = hz::aliased_load<uint64_t>(src_end - 56);
	auto tail2 = hz::aliased_load<uint64_t>(src_end - 48);
	auto tail3 = hz::aliased_load<uint64_t>(src_end - 40);
	auto tail4 = hz::aliased_load<uint64_t>(src_end - 32);
	auto tail5 = hz::aliased_load<uint64_t>(src_end - 24);
	auto tail6 = hz::aliased_load<uint64_t>(src_end - 16);
	auto tail7 = hz::aliased_load<uint64_t>(src_end - 8);

	auto align = hz::aliased_load<uint64_t>(src_ptr);
	auto* dest_align_ptr = dest_ptr;

	size_t align_amount = (8 - (reinterpret_cast<uintptr_t>(dest_ptr) & 0x7)) & 0x7;
	dest_ptr += align_amount;
	src_ptr += align_amount;
	size -= align_amount;

	size_t to_copy_blocks = (size - 1) / 64;

	if (reinterpret_cast<uintptr_t>(dest) - reinterpret_cast<uintptr_t>(src) >= orig_size) {
		for (size_t i = 0; i < to_copy_blocks; ++i) {
			auto* src_cur = src_ptr + i * 64;
			auto* dest_cur = dest_ptr + i * 64;

			auto m0 = hz::aliased_load<uint64_t>(src_cur);
			auto m1 = hz::aliased_load<uint64_t>(src_cur + 8);
			auto m2 = hz::aliased_load<uint64_t>(src_cur + 16);
			auto m3 = hz::aliased_load<uint64_t>(src_cur + 24);
			auto m4 = hz::aliased_load<uint64_t>(src_cur + 32);
			auto m5 = hz::aliased_load<uint64_t>(src_cur + 40);
			auto m6 = hz::aliased_load<uint64_t>(src_cur + 48);
			auto m7 = hz::aliased_load<uint64_t>(src_cur + 56);

			hz::aliased_store<uint64_t>(dest_cur, m0);
			hz::aliased_store<uint64_t>(dest_cur + 8, m1);
			hz::aliased_store<uint64_t>(dest_cur + 16, m2);
			hz::aliased_store<uint64_t>(dest_cur + 24, m3);
			hz::aliased_store<uint64_t>(dest_cur + 32, m4);
			hz::aliased_store<uint64_t>(dest_cur + 40, m5);
			hz::aliased_store<uint64_t>(dest_cur + 48, m6);
			hz::aliased_store<uint64_t>(dest_cur + 56, m7);
		}
	}
	else {
		for (size_t i = to_copy_blocks; i > 0; --i) {
			auto* src_cur = src_ptr + (i - 1) * 64;
			auto* dest_cur = dest_ptr + (i - 1) * 64;

			auto m0 = hz::aliased_load<uint64_t>(src_cur);
			auto m1 = hz::aliased_load<uint64_t>(src_cur + 8);
			auto m2 = hz::aliased_load<uint64_t>(src_cur + 16);
			auto m3 = hz::aliased_load<uint64_t>(src_cur + 24);
			auto m4 = hz::aliased_load<uint64_t>(src_cur + 32);
			auto m5 = hz::aliased_load<uint64_t>(src_cur + 40);
			auto m6 = hz::aliased_load<uint64_t>(src_cur + 48);
			auto m7 = hz::aliased_load<uint64_t>(src_cur + 56);

			hz::aliased_store<uint64_t>(dest_cur, m0);
			hz::aliased_store<uint64_t>(dest_cur + 8, m1);
			hz::aliased_store<uint64_t>(dest_cur + 16, m2);
			hz::aliased_store<uint64_t>(dest_cur + 24, m3);
			hz::aliased_store<uint64_t>(dest_cur + 32, m4);
			hz::aliased_store<uint64_t>(dest_cur + 40, m5);
			hz::aliased_store<uint64_t>(dest_cur + 48, m6);
			hz::aliased_store<uint64_t>(dest_cur + 56, m7);
		}
	}

	hz::aliased_store<uint64_t>(dest_align_ptr, align);
	hz::aliased_store<uint64_t>(dest_end - 64, tail0);
	hz::aliased_store<uint64_t>(dest_end - 56, tail1);
	hz::aliased_store<uint64_t>(dest_end - 48, tail2);
	hz::aliased_store<uint64_t>(dest_end - 40, tail3);
	hz::aliased_store<uint64_t>(dest_end - 32, tail4);
	hz::aliased_store<uint64_t>(dest_end - 24, tail5);
	hz::aliased_store<uint64_t>(dest_end - 16, tail6);
	hz::aliased_store<uint64_t>(dest_end - 8, tail7);

	return dest;
}
