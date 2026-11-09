#pragma once

#include "spinlock.hpp"
#include "atomic.hpp"

namespace hz {
	template<typename T, typename Lock = spinlock<void>>
	class seqlock {
	public:
		constexpr seqlock() = default;
		constexpr explicit seqlock(const T& value) : value_ {value} {}
		constexpr explicit seqlock(T&& value) : value_ {std::move(value)} {}

		seqlock(const seqlock&) = delete;
		seqlock& operator=(const seqlock&) = delete;

		[[nodiscard]] auto guarded_write_lock() {
			write_lock();
			return resource_guard {[this] {
				write_unlock();
			}, &value_};
		}

		void write_lock() {
			lock_.lock();
			unsigned int seq = sequence.load(memory_order::relaxed);
			sequence.store(seq + 1, memory_order::relaxed);
			// Order the sequence store before any data change.
			atomic_thread_fence(memory_order::release);
		}

		void write_unlock() {
			unsigned int seq = sequence.load(memory_order::relaxed);
			sequence.store(seq + 1, memory_order::release);
			lock_.unlock();
		}

		[[nodiscard]] unsigned int read_sequence() const {
			unsigned int seq;
#if defined(HZ_CPU_HAS_ARMED_LOAD) && defined(HZ_IN_KERNEL)
			auto* seq_ptr = sequence.data();
			while ((seq = armed_load_relaxed(seq_ptr)) & 1) {
				wait_for_event();
			}

			atomic_thread_fence(memory_order::acquire);
#else
			while ((seq = sequence.load(memory_order::acquire)) & 1) {
				spin_hint();
			}
#endif

			return seq;
		}

		T& read_unchecked() {
			return value_;
		}

		const T& read_unchecked() const {
			return value_;
		}

		struct seq_guard {
			[[nodiscard]] bool advanced() const {
				// Order the data loads before the sequence read.
				atomic_thread_fence(memory_order::acquire);
				return lock->sequence.load(memory_order::relaxed) != seq;
			}

		private:
			friend seqlock;

			constexpr seq_guard(const seqlock* lock, unsigned int seq) : lock {lock}, seq {seq} {}

			const seqlock* lock;
			unsigned int seq;
		};

		[[nodiscard]] seq_guard begin_read() const {
			return seq_guard {this, read_sequence()};
		}

		[[nodiscard]] T read() const {
			while (true) {
				auto guard = begin_read();
				T value = value_;
				if (!guard.advanced()) {
					return value;
				}
			}
		}

	private:
		T value_ {};
		hz::atomic<unsigned int> sequence;
		Lock lock_ {};
	};

	template<typename Lock>
	class seqlock<void, Lock> {
	public:
		constexpr seqlock() = default;

		seqlock(const seqlock&) = delete;
		seqlock& operator=(const seqlock&) = delete;

		[[nodiscard]] auto guarded_write_lock() {
			write_lock();
			return resource_guard {[this] {
				write_unlock();
			}};
		}

		void write_lock() {
			lock_.lock();
			unsigned int seq = sequence.load(memory_order::relaxed);
			sequence.store(seq + 1, memory_order::relaxed);
			// Order the sequence store before any data change.
			atomic_thread_fence(memory_order::release);
		}

		void write_unlock() {
			unsigned int seq = sequence.load(memory_order::relaxed);
			sequence.store(seq + 1, memory_order::release);
			lock_.unlock();
		}

		[[nodiscard]] unsigned int read_sequence() const {
			unsigned int seq;
#if defined(HZ_CPU_HAS_ARMED_LOAD) && defined(HZ_IN_KERNEL)
			auto* seq_ptr = sequence.data();
			while ((seq = armed_load_relaxed(seq_ptr)) & 1) {
				wait_for_event();
			}

			atomic_thread_fence(memory_order::acquire);
#else
			while ((seq = sequence.load(memory_order::acquire)) & 1) {
				spin_hint();
			}
#endif

			return seq;
		}

		struct seq_guard {
			[[nodiscard]] bool advanced() const {
				// Order the data loads before the sequence read.
				atomic_thread_fence(memory_order::acquire);
				return lock->sequence.load(memory_order::relaxed) != seq;
			}

		private:
			friend seqlock;

			constexpr seq_guard(const seqlock* lock, unsigned int seq) : lock {lock}, seq {seq} {}

			const seqlock* lock;
			unsigned int seq;
		};

		[[nodiscard]] seq_guard begin_read() const {
			return seq_guard {this, read_sequence()};
		}

	private:
		hz::atomic<unsigned int> sequence;
		Lock lock_ {};
	};
}
