#pragma once
#include "atomic.hpp"
#if __STDC_HOSTED__ == 1
#include <utility>
#else
#include "utility.hpp"
#endif

namespace hz {
	template<typename T>
	class spinlock {
	public:
		constexpr spinlock() = default;

		constexpr spinlock(T&& data) : data {.value {std::move(data)}, .lock {}} {} // NOLINT(*-explicit-constructor)
		constexpr spinlock(const T& data) : data {.value {data}, .lock {}} {} // NOLINT(*-explicit-constructor)

		struct guard {
			constexpr guard(const guard&) = delete;
			constexpr guard& operator=(const guard&) = delete;

			inline ~guard() {
				owner->data.lock.store(false, memory_order::release);
#ifdef __aarch64__
				asm volatile("sev");
#endif
			}

			operator T&() { // NOLINT(*-explicit-constructor)
				return owner->data.value;
			}

			T& operator*() {
				return owner->data.value;
			}

			T* operator->() {
				return &owner->data.value;
			}

		private:
			friend spinlock;

			constexpr explicit guard(spinlock* owner) : owner {owner} {}

			spinlock* owner;
		};

		[[nodiscard]] guard lock() {
			while (true) {
				if (!data.lock.exchange(true, memory_order::acquire)) {
					break;
				}
				while (data.lock.load(memory_order::relaxed)) {
#ifdef __x86_64__
					__builtin_ia32_pause();
#elif defined(__aarch64__)
					asm volatile("wfe");
#endif
				}
			}
			return guard {this};
		}

		T& get_unsafe() {
			return data.value;
		}

	private:
		struct Data {
			T value {};
			atomic<bool> lock {};
		};

		Data data {};
	};

	template<>
	class spinlock<void> {
	public:
		constexpr spinlock() = default;

		struct guard {
			constexpr guard(const guard&) = delete;
			constexpr guard& operator=(const guard&) = delete;

			inline ~guard() {
				owner->_lock.store(false, memory_order::release);
#ifdef __aarch64__
				asm volatile("sev");
#endif
			}

		private:
			friend spinlock;

			constexpr explicit guard(spinlock* owner) : owner {owner} {}

			spinlock* owner;
		};

		[[nodiscard]] guard lock() {
			while (true) {
				if (!_lock.exchange(true, memory_order::acquire)) {
					break;
				}
				while (_lock.load(memory_order::relaxed)) {
#ifdef __x86_64__
					__builtin_ia32_pause();
#elif defined(__aarch64__)
					asm volatile("wfe");
#endif
				}
			}
			return guard {this};
		}

	private:
		atomic<bool> _lock;
	};
}
