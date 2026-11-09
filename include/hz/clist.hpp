#pragma once

#include "container_of.hpp"

namespace hz {
	struct clist_hook {
		clist_hook* next;
		clist_hook* prev;

		constexpr void remove() {
			prev->next = next;
			next->prev = prev;
		}
	};

	template<typename T, clist_hook T::*Hook>
	class clist {
	public:
		template<typename U, bool Reverse>
		class iterator_base {
		public:
			U* operator*() {
				return container_of(ptr, Hook);
			}

			U* operator->() {
				return container_of(ptr, Hook);
			}

			constexpr bool operator==(iterator_base other) const {
				return ptr == other.ptr;
			}

			constexpr iterator_base& operator++() {
				if constexpr (Reverse) {
					ptr = ptr->prev;
				}
				else {
					ptr = ptr->next;
				}

				return *this;
			}

			constexpr iterator_base& operator--() {
				if constexpr (Reverse) {
					ptr = ptr->next;
				}
				else {
					ptr = ptr->prev;
				}

				return *this;
			}

		private:
			friend clist;

			constexpr explicit iterator_base(clist_hook* ptr) : ptr {ptr} {}

			clist_hook* ptr;
		};

		using iterator = iterator_base<T, false>;
		using const_iterator = iterator_base<const T, false>;
		using reverse_iterator = iterator_base<T, true>;
		using const_reverse_iterator = iterator_base<const T, true>;

		constexpr clist() = default;

		constexpr clist(clist&& other) noexcept {
			if (other.is_empty()) {
				return;
			}

			root = other.root;
			root.prev->next = &root;
			root.next->prev = &root;
			other.clear();
		}

		constexpr clist& operator=(clist&& other) noexcept {
			if (other.is_empty()) {
				clear();
				return *this;
			}

			root = other.root;
			root.prev->next = &root;
			root.next->prev = &root;
			other.clear();
			return *this;
		}

		clist(const clist&) = delete;
		clist& operator=(const clist&) = delete;

		constexpr void push_front(T* value) {
			insert_after_impl(&root, &(value->*Hook));
		}

		constexpr void push(T* value) {
			insert_before_impl(&root, &(value->*Hook));
		}

		constexpr T* pop_front() {
			if (root.next == &root) {
				return nullptr;
			}

			T* node = container_of(root.next, Hook);

			clist_hook* next = root.next->next;
			root.next = next;
			next->prev = &root;
			return node;
		}

		constexpr T* pop() {
			if (root.prev == &root) {
				return nullptr;
			}

			T* node = container_of(root.prev, Hook);

			clist_hook* prev = root.prev->prev;
			root.prev = prev;
			prev->next = &root;
			return node;
		}

		static constexpr iterator remove(T* value) {
			clist_hook* hook = &(value->*Hook);
			hook->prev->next = hook->next;
			hook->next->prev = hook->prev;
			return iterator {hook->next};
		}

		constexpr void insert(T* prev, T* value) {
			clist_hook* prev_hook = prev ? &(prev->*Hook) : &root;
			insert_after_impl(prev_hook, &(value->*Hook));
		}

		constexpr void insert_before(T* next, T* value) {
			clist_hook* next_hook = next ? &(next->*Hook) : &root;
			insert_before_impl(next_hook, &(value->*Hook));
		}

		constexpr void clear() {
			root.next = &root;
			root.prev = &root;
		}

		template<typename Compare>
		[[nodiscard]] constexpr T* find(Compare comp) {
			for (auto it = begin(); it != end(); ++it) {
				if (comp(*it)) {
					return *it;
				}
			}

			return nullptr;
		}

		template<typename Compare>
		[[nodiscard]] constexpr const T* find(Compare comp) const {
			for (auto it = begin(); it != end(); ++it) {
				if (comp(*it)) {
					return *it;
				}
			}

			return nullptr;
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return root.next == &root;
		}

		[[nodiscard]] constexpr iterator begin() {
			return iterator {root.next};
		}

		[[nodiscard]] constexpr const_iterator begin() const {
			return const_iterator {root.next};
		}

		[[nodiscard]] constexpr iterator end() {
			return iterator {&root};
		}

		[[nodiscard]] constexpr const_iterator end() const {
			return const_iterator {const_cast<clist_hook*>(&root)};
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() {
			return reverse_iterator {root.prev};
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const {
			return const_reverse_iterator {root.prev};
		}

		[[nodiscard]] constexpr reverse_iterator rend() {
			return reverse_iterator {&root};
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const {
			return const_reverse_iterator {const_cast<clist_hook*>(&root)};
		}

		[[nodiscard]] constexpr T* front() {
			return container_of(root.next, Hook);
		}

		[[nodiscard]] constexpr const T* front() const {
			return container_of(root.next, Hook);
		}

		[[nodiscard]] constexpr T* back() {
			return container_of(root.prev, Hook);
		}

		[[nodiscard]] constexpr const T* back() const {
			return container_of(root.prev, Hook);
		}

	private:
		static constexpr void insert_after_impl(clist_hook* after, clist_hook* value) {
			clist_hook* old = after->next;
			old->prev = value;
			after->next = value;

			value->prev = after;
			value->next = old;
		}

		static constexpr void insert_before_impl(clist_hook* before, clist_hook* value) {
			clist_hook* old = before->prev;
			old->next = value;
			before->prev = value;

			value->prev = old;
			value->next = before;
		}

		clist_hook root {&root, &root};
	};
}
