#pragma once

#include "atomic.hpp"
#include "resource_guard.hpp"
#include "cpu.hpp"

namespace hz {
	template<typename T>
	class spinlock {
	public:
		constexpr spinlock() = default;
		constexpr explicit spinlock(const T& value) : value_ {value} {}
		constexpr explicit spinlock(T&& value) : value_ {std::move(value)} {}

		spinlock(const spinlock&) = delete;
		spinlock& operator=(const spinlock&) = delete;

		[[nodiscard]] auto guarded_lock() {
			lock();
			return resource_guard {[this] {
				unlock();
			}, &value_};
		}

		void lock() {
			while (true) {
				if (!lock_.exchange(true, memory_order::acquire)) {
					break;
				}

#if defined(HZ_CPU_HAS_ARMED_LOAD) && defined(HZ_IN_KERNEL)
				auto* ptr = lock_.data();
				while (armed_load_relaxed(ptr)) {
					wait_for_event();
				}
#else
				while (lock_.load(memory_order::relaxed)) {
					spin_hint();
				}
#endif
			}
		}

		void unlock() {
			lock_.store(false, memory_order::release);
		}

	private:
		T value_ {};
		hz::atomic<bool> lock_;
	};

	template<>
	class spinlock<void> {
	public:
		constexpr spinlock() = default;

		constexpr spinlock(const spinlock&) {}
		constexpr spinlock(spinlock&&) noexcept {}

		constexpr spinlock& operator=(const spinlock&) {
			return *this;
		}

		constexpr spinlock& operator=(spinlock&&) noexcept {
			return *this;
		}

		[[nodiscard]] auto guarded_lock() {
			lock();
			return resource_guard {[this] {
				unlock();
			}};
		}

		void lock() {
			while (true) {
				if (!lock_.exchange(true, memory_order::acquire)) {
					break;
				}

#if defined(HZ_CPU_HAS_ARMED_LOAD) && defined(HZ_IN_KERNEL)
				auto* ptr = lock_.data();
				while (armed_load_relaxed(ptr)) {
					wait_for_event();
				}
#else
				while (lock_.load(memory_order::relaxed)) {
					spin_hint();
				}
#endif
			}
		}

		void unlock() {
			lock_.store(false, memory_order::release);
		}

	private:
		hz::atomic<bool> lock_;
	};
}
