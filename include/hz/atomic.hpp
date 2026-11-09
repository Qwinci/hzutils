#pragma once

#include <stddef.h>

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace hz {
	enum class memory_order : int {
#if defined(_MSC_VER) && !defined(__clang__)
		relaxed,
		consume,
		acquire,
		release,
		acq_rel,
		seq_cst
#else
		relaxed = __ATOMIC_RELAXED,
		consume = __ATOMIC_CONSUME,
		acquire = __ATOMIC_ACQUIRE,
		release = __ATOMIC_RELEASE,
		acq_rel = __ATOMIC_ACQ_REL,
		seq_cst = __ATOMIC_SEQ_CST
#endif
	};

#if defined(__clang__) || defined(__GNUC__)
	template<typename T>
	T atomic_load(const T* ptr, memory_order order) {
		T ret;
		__atomic_load(ptr, &ret, static_cast<int>(order));
		return ret;
	}

	template<typename T>
	void atomic_store(T* ptr, T value, memory_order order) {
		__atomic_store(ptr, &value, static_cast<int>(order));
	}

	template<typename T>
	T atomic_exchange(T* ptr, T value, memory_order order) {
		T ret;
		__atomic_exchange(ptr, &value, &ret, static_cast<int>(order));
		return ret;
	}

	template<typename T>
	bool atomic_compare_exchange(T* ptr, T& expected, T desired, memory_order success, memory_order failure) {
		__builtin_clear_padding(&expected);
		__builtin_clear_padding(&desired);
		return __atomic_compare_exchange(ptr, &expected, &desired, false, static_cast<int>(success), static_cast<int>(failure));
	}

	template<typename T>
	bool atomic_compare_exchange_weak(T* ptr, T& expected, T desired, memory_order success, memory_order failure) {
		__builtin_clear_padding(&expected);
		__builtin_clear_padding(&desired);
		return __atomic_compare_exchange(ptr, &expected, &desired, true, static_cast<int>(success), static_cast<int>(failure));
	}

	template<typename T>
	T atomic_fetch_add(T* ptr, T value, memory_order order) {
		return __atomic_fetch_add(ptr, value, static_cast<int>(order));
	}

	template<typename T>
	T atomic_fetch_sub(T* ptr, T value, memory_order order) {
		return __atomic_fetch_sub(ptr, value, static_cast<int>(order));
	}

	template<typename T>
	T atomic_fetch_and(T* ptr, T value, memory_order order) {
		return __atomic_fetch_and(ptr, value, static_cast<int>(order));
	}

	template<typename T>
	T atomic_fetch_or(T* ptr, T value, memory_order order) {
		return __atomic_fetch_or(ptr, value, static_cast<int>(order));
	}

	template<typename T>
	T atomic_fetch_xor(T* ptr, T value, memory_order order) {
		return __atomic_fetch_xor(ptr, value, static_cast<int>(order));
	}

	inline void atomic_signal_fence(memory_order order) {
		__atomic_signal_fence(static_cast<int>(order));
	}

	inline void atomic_thread_fence(memory_order order) {
		__atomic_thread_fence(static_cast<int>(order));
	}
#else
#ifdef _M_ARM64
#define __intrin_nf(name) name ## _nf
#define __intrin_acq(name) name ## _acq
#define __intrin_rel(name) name ## _rel
#else
#define __intrin_nf(name) name
#define __intrin_acq(name) name
#define __intrin_rel(name) name
#endif

	inline constexpr memory_order __atomic_cas_orders[6][6] = {
		// relaxed
		{memory_order::relaxed, memory_order::consume, memory_order::acquire, memory_order::release, memory_order::acq_rel, memory_order::seq_cst},
		// consume
		{memory_order::consume, memory_order::consume, memory_order::acquire, memory_order::acq_rel, memory_order::acq_rel, memory_order::seq_cst},
		// acquire
		{memory_order::acquire, memory_order::acquire, memory_order::acquire, memory_order::acq_rel, memory_order::acq_rel, memory_order::seq_cst},
		// release
		{memory_order::release, memory_order::acq_rel, memory_order::acq_rel, memory_order::release, memory_order::acq_rel, memory_order::seq_cst},
		// acq_rel
		{memory_order::acq_rel, memory_order::acq_rel, memory_order::acq_rel, memory_order::acq_rel, memory_order::acq_rel, memory_order::seq_cst},
		// seq_cst
		{memory_order::seq_cst, memory_order::seq_cst, memory_order::seq_cst, memory_order::seq_cst, memory_order::seq_cst, memory_order::seq_cst}
	};

	template<typename To, typename From>
	constexpr To __atomic_cast(const From& from) {
		return __builtin_bit_cast(To, from);
	}

	template<size_t S>
	struct __atomic_base;

	template<>
	struct __atomic_base<1> {
		using type = char;
		using utype = unsigned char;

		static type load(const void* ptr, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __iso_volatile_load8(static_cast<const type*>(ptr));
				case memory_order::consume:
				case memory_order::acquire:
				case memory_order::seq_cst:
				{
#ifdef _M_ARM64
					return static_cast<type>(__ldar8((unsigned __int8*) ptr));
#else
					type value = __iso_volatile_load8(static_cast<const type*>(ptr));
					_ReadWriteBarrier();
					return value;
#endif
				}
				case memory_order::release:
				case memory_order::acq_rel:
					__debugbreak();
					break;
			}

			__assume(false);
		}

		static void store(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					__iso_volatile_store8(static_cast<type*>(ptr), value);
					return;
				case memory_order::release:
#ifdef _M_ARM64
					__stlr8((unsigned __int8*) ptr, static_cast<unsigned __int8>(value));
#else
					_ReadWriteBarrier();
					__iso_volatile_store8(static_cast<type*>(ptr), value);
#endif
					return;
				case memory_order::consume:
				case memory_order::acquire:
				case memory_order::acq_rel:
					__debugbreak();
					break;
				case memory_order::seq_cst:
					_InterlockedExchange8(static_cast<type*>(ptr), value);
					return;
			}

			__assume(false);
		}

		static type exchange(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchange8)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchange8)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedExchange8)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchange8(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static bool compare_exchange(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			type* expected_ptr = static_cast<type*>(expected);
			type expected_val = *expected_ptr;
			type old;

			switch (__atomic_cas_orders[static_cast<int>(success)][static_cast<int>(failure)]) {
				case memory_order::relaxed:
					old = __intrin_nf(_InterlockedCompareExchange8)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::consume:
				case memory_order::acquire:
					old = __intrin_acq(_InterlockedCompareExchange8)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::release:
					old = __intrin_rel(_InterlockedCompareExchange8)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					old = _InterlockedCompareExchange8(static_cast<type*>(ptr), desired, expected_val);
					break;
				default:
					__assume(false);
			}

			if (old != expected_val) {
				*expected_ptr = old;
				return false;
			}
			else {
				return true;
			}
		}

		static bool compare_exchange_weak(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			return compare_exchange(ptr, expected, desired, success, failure);
		}

		static type fetch_add(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchangeAdd8)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchangeAdd8)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedExchangeAdd8)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchangeAdd8(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_sub(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchangeAdd8)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchangeAdd8)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::release:
					return __intrin_rel(_InterlockedExchangeAdd8)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchangeAdd8(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
			}

			__assume(false);
		}

		static type fetch_and(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedAnd8)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedAnd8)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedAnd8)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedAnd8(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_or(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedOr8)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedOr8)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedOr8)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedOr8(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_xor(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedXor8)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedXor8)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedXor8)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedXor8(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}
	};

	template<>
	struct __atomic_base<2> {
		using type = short;
		using utype = unsigned short;

		static type load(const void* ptr, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __iso_volatile_load16(static_cast<const type*>(ptr));
				case memory_order::consume:
				case memory_order::acquire:
				case memory_order::seq_cst:
				{
#ifdef _M_ARM64
					return static_cast<type>(__ldar16((unsigned __int16*) ptr));
#else
					type value = __iso_volatile_load16(static_cast<const type*>(ptr));
					_ReadWriteBarrier();
					return value;
#endif
				}
				case memory_order::release:
				case memory_order::acq_rel:
					__debugbreak();
					break;
			}

			__assume(false);
		}

		static void store(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					__iso_volatile_store16(static_cast<type*>(ptr), value);
					return;
				case memory_order::release:
#ifdef _M_ARM64
					__stlr16((unsigned __int16*) ptr, static_cast<unsigned __int16>(value));
#else
					_ReadWriteBarrier();
					__iso_volatile_store16(static_cast<type*>(ptr), value);
#endif
					return;
				case memory_order::consume:
				case memory_order::acquire:
				case memory_order::acq_rel:
					__debugbreak();
					break;
				case memory_order::seq_cst:
					_InterlockedExchange16(static_cast<type*>(ptr), value);
					return;
			}

			__assume(false);
		}

		static type exchange(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchange16)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchange16)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedExchange16)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchange16(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static bool compare_exchange(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			type* expected_ptr = static_cast<type*>(expected);
			type expected_val = *expected_ptr;
			type old;

			switch (__atomic_cas_orders[static_cast<int>(success)][static_cast<int>(failure)]) {
				case memory_order::relaxed:
					old = __intrin_nf(_InterlockedCompareExchange16)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::consume:
				case memory_order::acquire:
					old = __intrin_acq(_InterlockedCompareExchange16)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::release:
					old = __intrin_rel(_InterlockedCompareExchange16)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					old = _InterlockedCompareExchange16(static_cast<type*>(ptr), desired, expected_val);
					break;
				default:
					__assume(false);
			}

			if (old != expected_val) {
				*expected_ptr = old;
				return false;
			}
			else {
				return true;
			}
		}

		static bool compare_exchange_weak(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			return compare_exchange(ptr, expected, desired, success, failure);
		}

		static type fetch_add(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchangeAdd16)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchangeAdd16)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedExchangeAdd16)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchangeAdd16(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_sub(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchangeAdd16)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchangeAdd16)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::release:
					return __intrin_rel(_InterlockedExchangeAdd16)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchangeAdd16(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
			}

			__assume(false);
		}

		static type fetch_and(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedAnd16)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedAnd16)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedAnd16)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedAnd16(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_or(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedOr16)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedOr16)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedOr16)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedOr16(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_xor(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedXor16)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedXor16)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedXor16)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedXor16(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}
	};

	template<>
	struct __atomic_base<4> {
		using type = long;
		using utype = unsigned long;

		static type load(const void* ptr, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __iso_volatile_load32(static_cast<const int*>(ptr));
				case memory_order::consume:
				case memory_order::acquire:
				case memory_order::seq_cst:
				{
#ifdef _M_ARM64
					return static_cast<type>(__ldar32((unsigned __int32*) ptr));
#else
					type value = __iso_volatile_load32(static_cast<const int*>(ptr));
					_ReadWriteBarrier();
					return value;
#endif
				}
				case memory_order::release:
				case memory_order::acq_rel:
					__debugbreak();
					break;
			}

			__assume(false);
		}

		static void store(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					__iso_volatile_store32(static_cast<int*>(ptr), value);
					return;
				case memory_order::release:
#ifdef _M_ARM64
					__stlr32((unsigned __int32*) ptr, static_cast<unsigned __int32>(value));
#else
					_ReadWriteBarrier();
					__iso_volatile_store32(static_cast<int*>(ptr), value);
#endif
					return;
				case memory_order::consume:
				case memory_order::acquire:
				case memory_order::acq_rel:
					__debugbreak();
					break;
				case memory_order::seq_cst:
					_InterlockedExchange(static_cast<type*>(ptr), value);
					return;
			}

			__assume(false);
		}

		static type exchange(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchange)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchange)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedExchange)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchange(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static bool compare_exchange(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			type* expected_ptr = static_cast<type*>(expected);
			type expected_val = *expected_ptr;
			type old;

			switch (__atomic_cas_orders[static_cast<int>(success)][static_cast<int>(failure)]) {
				case memory_order::relaxed:
					old = __intrin_nf(_InterlockedCompareExchange)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::consume:
				case memory_order::acquire:
					old = __intrin_acq(_InterlockedCompareExchange)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::release:
					old = __intrin_rel(_InterlockedCompareExchange)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					old = _InterlockedCompareExchange(static_cast<type*>(ptr), desired, expected_val);
					break;
				default:
					__assume(false);
			}

			if (old != expected_val) {
				*expected_ptr = old;
				return false;
			}
			else {
				return true;
			}
		}

		static bool compare_exchange_weak(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			return compare_exchange(ptr, expected, desired, success, failure);
		}

		static type fetch_add(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchangeAdd)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchangeAdd)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedExchangeAdd)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchangeAdd(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_sub(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchangeAdd)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchangeAdd)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::release:
					return __intrin_rel(_InterlockedExchangeAdd)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchangeAdd(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
			}

			__assume(false);
		}

		static type fetch_and(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedAnd)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedAnd)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedAnd)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedAnd(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_or(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedOr)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedOr)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedOr)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedOr(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_xor(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedXor)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedXor)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedXor)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedXor(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}
	};

	template<>
	struct __atomic_base<8> {
		using type = long long;
		using utype = unsigned long long;

#ifndef _M_IX86
		static type load(const void* ptr, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __iso_volatile_load64(static_cast<const type*>(ptr));
				case memory_order::consume:
				case memory_order::acquire:
				case memory_order::seq_cst:
				{
#ifdef _M_ARM64
					return static_cast<type>(__ldar64((unsigned __int64*) ptr));
#else
					type value = __iso_volatile_load64(static_cast<const type*>(ptr));
					_ReadWriteBarrier();
					return value;
#endif
				}
				case memory_order::release:
				case memory_order::acq_rel:
					__debugbreak();
					break;
			}

			__assume(false);
		}

		static void store(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					__iso_volatile_store64(static_cast<type*>(ptr), value);
					return;
				case memory_order::release:
#ifdef _M_ARM64
					__stlr64((unsigned __int64*) ptr, static_cast<unsigned __int64>(value));
#else
					_ReadWriteBarrier();
					__iso_volatile_store64(static_cast<type*>(ptr), value);
#endif
					return;
				case memory_order::consume:
				case memory_order::acquire:
				case memory_order::acq_rel:
					__debugbreak();
					break;
				case memory_order::seq_cst:
					_InterlockedExchange64(static_cast<type*>(ptr), value);
					return;
			}

			__assume(false);
		}

		static type exchange(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchange64)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchange64)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedExchange64)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchange64(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

#endif

		static bool compare_exchange(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			type* expected_ptr = static_cast<type*>(expected);
			type expected_val = *expected_ptr;
			type old;

			switch (__atomic_cas_orders[static_cast<int>(success)][static_cast<int>(failure)]) {
				case memory_order::relaxed:
					old = __intrin_nf(_InterlockedCompareExchange64)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::consume:
				case memory_order::acquire:
					old = __intrin_acq(_InterlockedCompareExchange64)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::release:
					old = __intrin_rel(_InterlockedCompareExchange64)(static_cast<type*>(ptr), desired, expected_val);
					break;
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					old = _InterlockedCompareExchange64(static_cast<type*>(ptr), desired, expected_val);
					break;
				default:
					__assume(false);
			}

			if (old != expected_val) {
				*expected_ptr = old;
				return false;
			}
			else {
				return true;
			}
		}

		static bool compare_exchange_weak(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			return compare_exchange(ptr, expected, desired, success, failure);
		}

#ifndef _M_IX86
		static type fetch_add(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchangeAdd64)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchangeAdd64)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedExchangeAdd64)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchangeAdd64(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_sub(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedExchangeAdd64)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedExchangeAdd64)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::release:
					return __intrin_rel(_InterlockedExchangeAdd64)(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedExchangeAdd64(static_cast<type*>(ptr), static_cast<type>(~static_cast<utype>(value) + 1));
			}

			__assume(false);
		}

		static type fetch_and(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedAnd64)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedAnd64)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedAnd64)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedAnd64(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_or(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedOr64)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedOr64)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedOr64)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedOr64(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}

		static type fetch_xor(void* ptr, type value, memory_order order) {
			switch (order) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedXor64)(static_cast<type*>(ptr), value);
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedXor64)(static_cast<type*>(ptr), value);
				case memory_order::release:
					return __intrin_rel(_InterlockedXor64)(static_cast<type*>(ptr), value);
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedXor64(static_cast<type*>(ptr), value);
			}

			__assume(false);
		}
#endif
	};

#if defined(_M_AMD64) || defined(_M_ARM64)
	template<>
	struct __atomic_base<16> {
		struct placeholder {
			long long low;
			long long high;
		};

		using type = placeholder;

		static bool compare_exchange(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			switch (__atomic_cas_orders[static_cast<int>(success)][static_cast<int>(failure)]) {
				case memory_order::relaxed:
					return __intrin_nf(_InterlockedCompareExchange128)(static_cast<long long*>(ptr), desired.high, desired.low, static_cast<long long*>(expected));
				case memory_order::consume:
				case memory_order::acquire:
					return __intrin_acq(_InterlockedCompareExchange128)(static_cast<long long*>(ptr), desired.high, desired.low, static_cast<long long*>(expected));
				case memory_order::release:
					return __intrin_rel(_InterlockedCompareExchange128)(static_cast<long long*>(ptr), desired.high, desired.low, static_cast<long long*>(expected));
				case memory_order::acq_rel:
				case memory_order::seq_cst:
					return _InterlockedCompareExchange128(static_cast<long long*>(ptr), desired.high, desired.low, static_cast<long long*>(expected));
			}

			__assume(false);
		}

		static bool compare_exchange_weak(void* ptr, void* expected, type desired, memory_order success, memory_order failure) {
			return compare_exchange(ptr, expected, desired, success, failure);
		}
	};
#endif

	template<typename T>
	T atomic_load(const T* ptr, memory_order order) {
		return __atomic_cast<T>(__atomic_base<sizeof(T)>::load(ptr, order));
	}

	template<typename T>
	void atomic_store(T* ptr, T value, memory_order order) {
		using base = __atomic_base<sizeof(T)>;
		base::store(ptr, __atomic_cast<typename base::type>(value), order);
	}

	template<typename T>
	T atomic_exchange(T* ptr, T value, memory_order order) {
		using base = __atomic_base<sizeof(T)>;
		return __atomic_cast<T>(base::exchange(ptr, __atomic_cast<typename base::type>(value), order));
	}

	template<typename T>
	bool atomic_compare_exchange(T* ptr, T& expected, T desired, memory_order success, memory_order failure) {
		using base = __atomic_base<sizeof(T)>;
		__builtin_zero_non_value_bits(&expected);
		__builtin_zero_non_value_bits(&desired);
		return base::compare_exchange(ptr, &expected, __atomic_cast<typename base::type>(desired), success, failure);
	}

	template<typename T>
	bool atomic_compare_exchange_weak(T* ptr, T& expected, T desired, memory_order success, memory_order failure) {
		using base = __atomic_base<sizeof(T)>;
		__builtin_zero_non_value_bits(&expected);
		__builtin_zero_non_value_bits(&desired);
		return base::compare_exchange_weak(ptr, &expected, __atomic_cast<typename base::type>(desired), success, failure);
	}

	template<typename T>
	T atomic_fetch_add(T* ptr, T value, memory_order order) {
		using base = __atomic_base<sizeof(T)>;
		return __atomic_cast<T>(base::fetch_add(ptr, __atomic_cast<typename base::type>(value), order));
	}

	template<typename T>
	T atomic_fetch_sub(T* ptr, T value, memory_order order) {
		using base = __atomic_base<sizeof(T)>;
		return __atomic_cast<T>(base::fetch_sub(ptr, __atomic_cast<typename base::type>(value), order));
	}

	template<typename T>
	T atomic_fetch_and(T* ptr, T value, memory_order order) {
		using base = __atomic_base<sizeof(T)>;
		return __atomic_cast<T>(base::fetch_and(ptr, __atomic_cast<typename base::type>(value), order));
	}

	template<typename T>
	T atomic_fetch_or(T* ptr, T value, memory_order order) {
		using base = __atomic_base<sizeof(T)>;
		return __atomic_cast<T>(base::fetch_or(ptr, __atomic_cast<typename base::type>(value), order));
	}

	template<typename T>
	T atomic_fetch_xor(T* ptr, T value, memory_order order) {
		using base = __atomic_base<sizeof(T)>;
		return __atomic_cast<T>(base::fetch_xor(ptr, __atomic_cast<typename base::type>(value), order));
	}

	inline void atomic_signal_fence(memory_order order) {
		if (order != memory_order::relaxed) {
			_ReadWriteBarrier();
		}
	}

	inline void atomic_thread_fence(memory_order order) {
		if (order == memory_order::relaxed) {
			return;
		}

#if defined(_M_IX86) || defined(_M_AMD64)
		_ReadWriteBarrier();
		if (order == memory_order::seq_cst) {
			volatile long dummy;
			(void) _InterlockedIncrement(&dummy);
			_ReadWriteBarrier();
		}
#elif defined(_M_ARM64)
		if (order == memory_order::acquire || order == memory_order::consume) {
			// dmb ishld
			__dmb(9);
		}
		else {
			// dmb ish
			__dmb(0xB);
		}
#endif
	}

#undef __intrin_nf
#undef __intrin_acq
#undef __intrin_rel
#endif

	template<typename T>
	class atomic {
	public:
		constexpr atomic() = default;
		constexpr explicit atomic(T value) : value_ {value} {}

		atomic(const atomic&) = delete;
		atomic& operator=(const atomic&) = delete;

		T load(memory_order order) const {
			return atomic_load(&value_, order);
		}

		void store(T value, memory_order order) {
			atomic_store(&value_, value, order);
		}

		T exchange(T value, memory_order order) {
			return atomic_exchange(&value_, value, order);
		}

		bool compare_exchange(T& expected, T desired, memory_order success, memory_order failure) {
			return atomic_compare_exchange(&value_, expected, desired, success, failure);
		}

		bool compare_exchange_weak(T& expected, T desired, memory_order success, memory_order failure) {
			return atomic_compare_exchange_weak(&value_, expected, desired, success, failure);
		}

		T fetch_add(T value, memory_order order) {
			return atomic_fetch_add(&value_, value, order);
		}

		T fetch_sub(T value, memory_order order) {
			return atomic_fetch_sub(&value_, value, order);
		}

		T fetch_and(T value, memory_order order) {
			return atomic_fetch_and(&value_, value, order);
		}

		T fetch_or(T value, memory_order order) {
			return atomic_fetch_or(&value_, value, order);
		}

		T fetch_xor(T value, memory_order order) {
			return atomic_fetch_xor(&value_, value, order);
		}

		[[nodiscard]] T* data() {
			return &value_;
		}

		[[nodiscard]] constexpr const T* data() const {
			return &value_;
		}

	private:
		T value_ {};
	};
}
