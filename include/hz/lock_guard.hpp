#pragma once

namespace hz {
	struct adopt_lock_t {
		explicit adopt_lock_t() = default;
	};

	inline constexpr adopt_lock_t adopt_lock {};

	// RAII lock guard
	// Note: The destructor always calls unlock() so the lock must be locked when the guard is destroyed.
	template<typename T>
	class [[nodiscard]] lock_guard {
	public:
		constexpr explicit lock_guard(T& lock) : lock_ {lock} {
			lock_.lock();
		}

		constexpr lock_guard(adopt_lock_t, T& lock) : lock_ {lock} {}

		lock_guard(const lock_guard&) = delete;
		lock_guard& operator=(const lock_guard&) = delete;

		constexpr void lock() {
			lock_.lock();
		}

		constexpr void unlock() {
			lock_.unlock();
		}

		constexpr ~lock_guard() {
			lock_.unlock();
		}

	private:
		T& lock_;
	};

	template<typename T>
	[[nodiscard]] constexpr lock_guard<T> guard(T& lock) {
		return lock_guard<T> {lock};
	}
}
