#pragma once

namespace hz {
	struct list_hook {
		void* prev {};
		void* next {};
	};

	template<typename T, list_hook (T::*Hook)>
	class list {
	public:
		class iterator {
		private:
			constexpr explicit iterator(T* ptr) : ptr {ptr}, next {ptr ? static_cast<T*>((ptr->*Hook).next) : nullptr} {}

			T* ptr;
			T* next;
			friend class list;
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
			friend class list;
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
			(value->*Hook).prev = nullptr;
			(value->*Hook).next = root;
			if (root) {
				(root->*Hook).prev = value;
			}
			else {
				_end = value;
			}
			root = value;
		}

		constexpr void push(T* value) {
			(value->*Hook).prev = _end;
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
			if (root) {
				(root->*Hook).prev = nullptr;
			}
			else {
				_end = nullptr;
			}
			return node;
		}

		constexpr T* pop() {
			if (!_end) {
				return nullptr;
			}
			auto node = _end;
			_end = static_cast<T*>((node->*Hook).prev);
			if (_end) {
				(_end->*Hook).next = nullptr;
			}
			else {
				root = nullptr;
			}
			return node;
		}

		constexpr void remove(T* value) {
			auto prev = static_cast<T*>((value->*Hook).prev);
			if (prev) {
				(prev->*Hook).next = (value->*Hook).next;
			}
			else {
				root = static_cast<T*>((value->*Hook).next);
			}

			auto next = static_cast<T*>((value->*Hook).next);
			if (next) {
				(next->*Hook).prev = prev;
			}
			else {
				_end = prev;
			}
		}

		constexpr void insert(T* prev, T* value) {
			(value->*Hook).prev = prev;

			if (prev) {
				(value->*Hook).next = (prev->*Hook).next;
				(prev->*Hook).next = value;
			}
			else {
				(value->*Hook).next = root;
				root = value;
			}

			if ((value->*Hook).next) {
				(static_cast<T*>((value->*Hook).next)->*Hook).prev = value;
			}
			else {
				_end = value;
			}
		}

		constexpr void insert_before(T* next, T* value) {
			(value->*Hook).next = next;

			if (next) {
				(value->*Hook).prev = (next->*Hook).prev;
				(next->*Hook).prev = value;

				if ((value->*Hook).prev) {
					(static_cast<T*>((value->*Hook).prev)->*Hook).next = value;
				}
				else {
					root = value;
				}
			}
			else {
				(value->*Hook).prev = nullptr;
				(value->*Hook).next = root;
				if (root) {
					(root->*Hook).prev = value;
				}
				else {
					_end = value;
				}
				root = value;
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
