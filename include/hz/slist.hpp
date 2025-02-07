#pragma once

namespace hz {
	struct slist_hook {
		void* next {};
	};

	template<typename T, slist_hook (T::*Hook)>
	class slist {
	public:
		class iterator {
		private:
			constexpr explicit iterator(T* ptr) : ptr {ptr}, next {ptr ? static_cast<T*>((ptr->*Hook).next) : nullptr} {}

			T* ptr;
			T* next;
			friend class slist;
		public:
			constexpr bool operator!=(const iterator& other) const {
				return ptr != other.ptr;
			}

			constexpr iterator& operator++() {
				ptr = next;
				if (ptr) {
					next = static_cast<T*>((ptr->*Hook).next);
				}

				return *this;
			}

			constexpr T& operator*() {
				return *ptr;
			}
		};

		class const_iterator {
		private:
			constexpr explicit const_iterator(const T* ptr) : ptr {ptr} {}

			const T* ptr;
			friend class slist;
		public:
			constexpr bool operator!=(const iterator& other) const {
				return ptr != other.ptr;
			}

			constexpr iterator& operator++() {
				ptr = static_cast<const T*>((ptr->*Hook).next);
				return *this;
			}

			constexpr const T& operator*() const {
				return *ptr;
			}
		};

		constexpr void push_front(T* value) {
			(value->*Hook).next = root;
			if (!root) {
				_end = value;
			}
			root = value;
		}

		constexpr void push(T* value) {
			(value->*Hook).next = nullptr;
			if (_end) {
				(_end->*Hook).next = value;
			}
			else {
				root = value;
			}
			_end = value;
		}

		constexpr T* pop_front() {
			if (!root) {
				return nullptr;
			}
			auto node = root;
			root = static_cast<T*>((node->*Hook).next);
			if (!root) {
				_end = nullptr;
			}
			return node;
		}

		constexpr void insert(T* prev, T* value) {
			if (prev) {
				(value->*Hook).next = (prev->*Hook).next;
				(prev->*Hook).next = value;
			}
			else {
				(value->*Hook).next = root;
				root = value;
			}

			if (!(value->*Hook).next) {
				_end = value;
			}
		}

		constexpr void clear() {
			root = nullptr;
			_end = nullptr;
		}

		template<typename Compare>
		[[nodiscard]] constexpr T* find(Compare comp) {
			for (auto iter = begin(); iter != end(); ++iter) {
				if (comp(*iter)) {
					return &*iter;
				}
			}
			return nullptr;
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return !root;
		}

		[[nodiscard]] constexpr iterator begin() {
			return iterator {root};
		}

		[[nodiscard]] constexpr iterator end() {
			return iterator {nullptr};
		}

		[[nodiscard]] constexpr const_iterator begin() const {
			return const_iterator {root};
		}

		[[nodiscard]] constexpr const_iterator end() const {
			return const_iterator {nullptr};
		}

		[[nodiscard]] constexpr T* front() const {
			return root;
		}

		[[nodiscard]] constexpr T* back() const {
			return _end;
		}

	private:
		T* root {};
		T* _end {};
	};
}
