#include "alias_helper.hpp"

#include <emmintrin.h>
#include <stddef.h>
#include <stdint.h>

#ifndef HZ_MEM_SSE2_PREFIX
#define HZ_MEM_SSE2_PREFIX

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

#define HZ_MEM_FUNC(name) CONCAT(HZ_MEM_SSE2_PREFIX, name)

static __m128i loadu(const unsigned char* ptr) {
	return _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
}

static void storeu(unsigned char* ptr, __m128i value) {
	_mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), value);
}

static void store(unsigned char* ptr, __m128i value) {
	_mm_store_si128(reinterpret_cast<__m128i*>(ptr), value);
}

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

	__m128i xmm_mask = _mm_set1_epi64x(static_cast<int64_t>(mask));

	if (size <= 32) {
		storeu(dest_ptr, xmm_mask);
		storeu(dest_ptr + size - 16, xmm_mask);
		return dest;
	}

	uintptr_t misalign = reinterpret_cast<uintptr_t>(dest) & 0xF;
	if (misalign) {
		storeu(dest_ptr, xmm_mask);
		dest_ptr += 16 - misalign;
		size -= 16 - misalign;

		if (size <= 32) {
			store(dest_ptr, xmm_mask);
			storeu(dest_ptr + size - 16, xmm_mask);
			return dest;
		}
	}

	if (size <= 64) {
		store(dest_ptr, xmm_mask);
		store(dest_ptr + 16, xmm_mask);
		storeu(dest_ptr + size - 32, xmm_mask);
		storeu(dest_ptr + size - 16, xmm_mask);
		return dest;
	}

	auto* end = dest_ptr + size;

	while (size > 64) {
		store(dest_ptr, xmm_mask);
		store(dest_ptr + 16, xmm_mask);
		store(dest_ptr + 32, xmm_mask);
		store(dest_ptr + 48, xmm_mask);
		dest_ptr += 64;
		size -= 64;
	}

	storeu(end - 64, xmm_mask);
	storeu(end - 48, xmm_mask);
	storeu(end - 32, xmm_mask);
	storeu(end - 16, xmm_mask);

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
		__m128i first = loadu(src_ptr);
		__m128i second = loadu(src_ptr + size - 16);

		storeu(dest_ptr, first);
		storeu(dest_ptr + size - 16, second);
		return dest;
	}

	uintptr_t misalign = reinterpret_cast<uintptr_t>(dest) & 0xF;
	if (misalign) {
		storeu(dest_ptr, loadu(src_ptr));
		dest_ptr += 16 - misalign;
		src_ptr += 16 - misalign;
		size -= 16 - misalign;

		if (size <= 32) {
			__m128i first = loadu(src_ptr);
			__m128i second = loadu(src_ptr + size - 16);
			store(dest_ptr, first);
			storeu(dest_ptr + size - 16, second);
			return dest;
		}
	}

	if (size <= 64) {
		__m128i first = loadu(src_ptr);
		__m128i second = loadu(src_ptr + 16);
		__m128i third = loadu(src_ptr + size - 32);
		__m128i fourth = loadu(src_ptr + size - 16);

		store(dest_ptr, first);
		store(dest_ptr + 16, second);
		storeu(dest_ptr + size - 32, third);
		storeu(dest_ptr + size - 16, fourth);
		return dest;
	}

	while (size > 64) {
		__m128i first = loadu(src_ptr);
		__m128i second = loadu(src_ptr + 16);
		__m128i third = loadu(src_ptr + 32);
		__m128i fourth = loadu(src_ptr + 48);

		store(dest_ptr, first);
		store(dest_ptr + 16, second);
		store(dest_ptr + 32, third);
		store(dest_ptr + 48, fourth);

		dest_ptr += 64;
		src_ptr += 64;
		size -= 64;
	}

	__m128i first = loadu(src_ptr + size - 64);
	__m128i second = loadu(src_ptr + size - 48);
	__m128i third = loadu(src_ptr + size - 32);
	__m128i fourth = loadu(src_ptr + size - 16);

	storeu(dest_ptr + size - 64, first);
	storeu(dest_ptr + size - 48, second);
	storeu(dest_ptr + size - 32, third);
	storeu(dest_ptr + size - 16, fourth);

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
		__m128i first = loadu(src_ptr);
		__m128i second = loadu(src_ptr + size - 16);

		storeu(dest_ptr, first);
		storeu(dest_ptr + size - 16, second);
		return dest;
	}

	if (size <= 64) {
		__m128i first = loadu(src_ptr);
		__m128i second = loadu(src_ptr + 16);
		__m128i third = loadu(src_ptr + size - 32);
		__m128i fourth = loadu(src_ptr + size - 16);

		storeu(dest_ptr, first);
		storeu(dest_ptr + 16, second);
		storeu(dest_ptr + size - 32, third);
		storeu(dest_ptr + size - 16, fourth);
		return dest;
	}

	size_t orig_size = size;

	auto* dest_end = dest_ptr + size;
	auto* src_end = src_ptr + size;

	__m128i tail0 = loadu(src_end - 64);
	__m128i tail1 = loadu(src_end - 48);
	__m128i tail2 = loadu(src_end - 32);
	__m128i tail3 = loadu(src_end - 16);

	__m128i align = loadu(src_ptr);
	auto* dest_align_ptr = dest_ptr;

	size_t align_amount = (16 - (reinterpret_cast<uintptr_t>(dest_ptr) & 0xF)) & 0xF;
	dest_ptr += align_amount;
	src_ptr += align_amount;
	size -= align_amount;

	size_t to_copy_blocks = (size - 1) / 64;

	if (reinterpret_cast<uintptr_t>(dest) - reinterpret_cast<uintptr_t>(src) >= orig_size) {
		for (size_t i = 0; i < to_copy_blocks; ++i) {
			auto* src_cur = src_ptr + i * 64;
			auto* dest_cur = dest_ptr + i * 64;

			__m128i first = loadu(src_cur);
			__m128i second = loadu(src_cur + 16);
			__m128i third = loadu(src_cur + 32);
			__m128i fourth = loadu(src_cur + 48);

			store(dest_cur, first);
			store(dest_cur + 16, second);
			store(dest_cur + 32, third);
			store(dest_cur + 48, fourth);
		}
	}
	else {
		for (size_t i = to_copy_blocks; i > 0; --i) {
			auto* src_cur = src_ptr + (i - 1) * 64;
			auto* dest_cur = dest_ptr + (i - 1) * 64;

			__m128i first = loadu(src_cur);
			__m128i second = loadu(src_cur + 16);
			__m128i third = loadu(src_cur + 32);
			__m128i fourth = loadu(src_cur + 48);

			store(dest_cur, first);
			store(dest_cur + 16, second);
			store(dest_cur + 32, third);
			store(dest_cur + 48, fourth);
		}
	}

	storeu(dest_align_ptr, align);
	storeu(dest_end - 64, tail0);
	storeu(dest_end - 48, tail1);
	storeu(dest_end - 32, tail2);
	storeu(dest_end - 16, tail3);

	return dest;
}
