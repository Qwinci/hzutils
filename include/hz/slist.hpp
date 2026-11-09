#pragma once

namespace hz {
	template<typename T>
	struct slist_hook {
		T* next;
	};

	template<typename T, slist_hook<T> T::*Hook>
	class slist {
	public:
		template<typename U>
		class iterator_base {
		public:
			constexpr bool operator==(iterator_base other) const {
				return ptr == other.ptr;
			}

			constexpr iterator_base& operator++() {
				ptr = (ptr->*Hook).next;
				return *this;
			}

			constexpr U& operator*() {
				return *ptr;
			}

		private:
			friend slist;

			constexpr explicit iterator_base(U* ptr) : ptr {ptr} {}

			U* ptr;
		};

		using iterator = iterator_base<T>;
		using const_iterator = iterator_base<const T>;

		constexpr void push_front(T* value) {
			(value->*Hook).next = root;
			if (!root) {
				end_ = value;
			}
			root = value;
		}

		constexpr void push(T* value) {
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
			if (!root) {
				end_ = nullptr;
			}
			return node;
		}

		constexpr void remove_after(T* prev, T* value) {
			if (prev) {
				(prev->*Hook).next = (value->*Hook).next;
			}
			else {
				root = (value->*Hook).next;
			}

			if (!(value->*Hook).next) {
				end_ = prev;
			}
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
				end_ = value;
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
					return &*iter;
				}
			}

			return nullptr;
		}

		template<typename Compare>
		[[nodiscard]] constexpr const T* find(Compare comp) const {
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

		[[nodiscard]] constexpr const_iterator begin() const {
			return const_iterator {root};
		}

		[[nodiscard]] constexpr iterator end() {
			return iterator {nullptr};
		}

		[[nodiscard]] constexpr const_iterator end() const {
			return const_iterator {nullptr};
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
