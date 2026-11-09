#pragma once

namespace hz {
	template<typename T>
	struct list_hook {
		T* prev;
		T* next;
	};

	template<typename T, list_hook<T> T::*Hook>
	class list {
	public:
		template<typename U, bool Reverse>
		class iterator_base {
		public:
			constexpr U* operator*() {
				return ptr;
			}

			constexpr U* operator->() {
				return ptr;
			}

			constexpr bool operator==(iterator_base other) const {
				return ptr == other.ptr;
			}

			constexpr iterator_base& operator++() {
				if constexpr (Reverse) {
					ptr = (ptr->*Hook).prev;
				}
				else {
					ptr = (ptr->*Hook).next;
				}

				return *this;
			}

			constexpr iterator_base& operator--() {
				if constexpr (Reverse) {
					ptr = ptr ? (ptr->*Hook).next : l->root;
				}
				else {
					ptr = ptr ? (ptr->*Hook).prev : l->end_;
				}

				return *this;
			}

		private:
			friend list;

			constexpr explicit iterator_base(const list* l, U* ptr) : l {l}, ptr {ptr} {}

			const list* l;
			U* ptr;
		};

		using iterator = iterator_base<T, false>;
		using const_iterator = iterator_base<const T, false>;
		using reverse_iterator = iterator_base<T, true>;
		using const_reverse_iterator = iterator_base<const T, true>;

		constexpr void push_front(T* value) {
			(value->*Hook).prev = nullptr;
			(value->*Hook).next = root;
			if (root) {
				(root->*Hook).prev = value;
			}
			else {
				end_ = value;
			}
			root = value;
		}

		constexpr void push(T* value) {
			(value->*Hook).prev = end_;
			(value->*Hook).next = nullptr;
			if (end_) {
				(end_->*Hook).next = value;
			}
			else {
				root = value;
			}
			end_ = value;
		}

		constexpr T* pop_front() {
			if (!root) {
				return nullptr;
			}
			T* node = root;
			root = (node->*Hook).next;
			if (root) {
				(root->*Hook).prev = nullptr;
			}
			else {
				end_ = nullptr;
			}
			return node;
		}

		constexpr T* pop() {
			if (!end_) {
				return nullptr;
			}
			T* node = end_;
			end_ = (node->*Hook).prev;
			if (end_) {
				(end_->*Hook).next = nullptr;
			}
			else {
				root = nullptr;
			}
			return node;
		}

		constexpr iterator remove(T* value) {
			T* prev = (value->*Hook).prev;
			T* next = (value->*Hook).next;

			if (prev) {
				(prev->*Hook).next = next;
			}
			else {
				root = next;
			}

			if (next) {
				(next->*Hook).prev = prev;
			}
			else {
				end_ = prev;
			}

			return iterator {this, next};
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

			if (auto* next = (value->*Hook).next) {
				(next->*Hook).prev = value;
			}
			else {
				end_ = value;
			}
		}

		constexpr void insert_before(T* next, T* value) {
			(value->*Hook).next = next;

			if (next) {
				(value->*Hook).prev = (next->*Hook).prev;
				(next->*Hook).prev = value;

				if (auto prev = (value->*Hook).prev) {
					(prev->*Hook).next = value;
				}
				else {
					root = value;
				}
			}
			else {
				push(value);
			}
		}

		constexpr void clear() {
			root = nullptr;
			end_ = nullptr;
		}

		template<typename Compare>
		[[nodiscard]] constexpr T* find(Compare comp) {
			for (auto iter = begin(); iter != end(); ++iter) {
				if (comp(*iter)) {
					return *iter;
				}
			}

			return nullptr;
		}

		template<typename Compare>
		[[nodiscard]] constexpr const T* find(Compare comp) const {
			for (auto iter = begin(); iter != end(); ++iter) {
				if (comp(*iter)) {
					return *iter;
				}
			}

			return nullptr;
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return !root;
		}

		[[nodiscard]] constexpr iterator begin() {
			return iterator {this, root};
		}

		[[nodiscard]] constexpr const_iterator begin() const {
			return const_iterator {this, root};
		}

		[[nodiscard]] constexpr iterator end() {
			return iterator {this, nullptr};
		}

		[[nodiscard]] constexpr const_iterator end() const {
			return const_iterator {this, nullptr};
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() {
			return reverse_iterator {this, end_};
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const {
			return const_reverse_iterator {this, end_};
		}

		[[nodiscard]] constexpr reverse_iterator rend() {
			return reverse_iterator {this, nullptr};
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const {
			return const_reverse_iterator {this, nullptr};
		}

		[[nodiscard]] constexpr T* front() {
			return root;
		}

		[[nodiscard]] constexpr const T* front() const {
			return root;
		}

		[[nodiscard]] constexpr T* back() {
			return end_;
		}

		[[nodiscard]] constexpr const T* back() const {
			return end_;
		}

	private:
		T* root {};
		T* end_ {};
	};
}
