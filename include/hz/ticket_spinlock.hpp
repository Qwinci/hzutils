#pragma once

#include "atomic.hpp"
#include "resource_guard.hpp"
#include "cpu.hpp"

namespace hz {
	template<typename T>
	class ticket_spinlock {
	public:
		constexpr ticket_spinlock() = default;
		constexpr explicit ticket_spinlock(const T& value) : value_ {value} {}
		constexpr explicit ticket_spinlock(T&& value) : value_ {std::move(value)} {}

		ticket_spinlock(const ticket_spinlock&) = delete;
		ticket_spinlock& operator=(const ticket_spinlock&) = delete;

		[[nodiscard]] auto guarded_lock() {
			lock();
			return resource_guard {[this] {
				unlock();
			}, &value_};
		}

		void lock() {
			unsigned int ticket = next.fetch_add(1, memory_order::acquire);

#if defined(HZ_CPU_HAS_ARMED_LOAD) && defined(HZ_IN_KERNEL)
			auto* ptr = serving.data();
			while (armed_load_relaxed(ptr) != ticket) {
				wait_for_event();
			}
#else
			while (serving.load(memory_order::relaxed) != ticket) {
				spin_hint();
			}
#endif

			atomic_thread_fence(memory_order::acquire);
		}

		void unlock() {
			serving.fetch_add(1, memory_order::release);
		}

	private:
		T value_ {};
		hz::atomic<unsigned int> serving;
		hz::atomic<unsigned int> next;
	};

	template<>
	class ticket_spinlock<void> {
	public:
		constexpr ticket_spinlock() = default;

		constexpr ticket_spinlock(const ticket_spinlock&) {}
		constexpr ticket_spinlock(ticket_spinlock&&) noexcept {}

		constexpr ticket_spinlock& operator=(const ticket_spinlock&) {
			return *this;
		}

		constexpr ticket_spinlock& operator=(ticket_spinlock&&) noexcept {
			return *this;
		}

		[[nodiscard]] auto guarded_lock() {
			lock();
			return resource_guard {[this] {
				unlock();
			}};
		}

		void lock() {
			unsigned int ticket = next.fetch_add(1, memory_order::acquire);

#if defined(HZ_CPU_HAS_ARMED_LOAD) && defined(HZ_IN_KERNEL)
			auto* ptr = serving.data();
			while (armed_load_relaxed(ptr) != ticket) {
				wait_for_event();
			}
#else
			while (serving.load(memory_order::relaxed) != ticket) {
				spin_hint();
			}
#endif

			atomic_thread_fence(memory_order::acquire);
		}

		void unlock() {
			serving.fetch_add(1, memory_order::release);
		}

	private:
		hz::atomic<unsigned int> serving;
		hz::atomic<unsigned int> next;
	};
}
