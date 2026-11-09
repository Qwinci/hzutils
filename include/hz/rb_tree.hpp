#pragma once

#include <stdint.h>

#include "config.hpp"

#if HZ_FREESTANDING
#include "compare.hpp"
#else
#include <compare>
#endif

namespace hz {
	template<typename T>
	struct rb_hook {
		T* get_parent() const {
			return reinterpret_cast<T*>(parent & ~1);
		}

		T* get_left() const {
			return children[0];
		}

		T* get_right() const {
			return children[1];
		}

	private:
		template<typename T2, rb_hook<T2> T2::*Hook, typename Augments>
		friend class rb_tree;

		void set_parent(T* new_parent) {
			parent = reinterpret_cast<uintptr_t>(new_parent) | (parent & 1);
		}

		void set_color(bool red) {
			if (red) {
				parent |= 1;
			}
			else {
				parent &= ~1;
			}
		}

		void set_parent_color(T* new_parent, bool red) {
			parent = reinterpret_cast<uintptr_t>(new_parent) | static_cast<uintptr_t>(red);
		}

		[[nodiscard]] bool is_red() const {
			return parent & 1;
		}

		uintptr_t parent {};

	public:
		T* children[2] {};
	};

	struct rb_tree_default_augments {};

	template<typename T, rb_hook<T> T::*Hook, typename Augments = rb_tree_default_augments>
	class rb_tree {
	public:
		template<bool Reverse>
		struct iterator_base {
			T* operator*() const {
				return node;
			}

			T* operator->() const {
				return node;
			}

			bool operator==(const iterator_base& other) const {
				return node == other.node;
			}

			iterator_base& operator++() {
				if constexpr (Reverse) {
					node = predecessor(node);
				}
				else {
					node = successor(node);
				}

				return *this;
			}

			iterator_base& operator--() {
				if constexpr (Reverse) {
					node = node ? successor(node) : minimum(tree->root);
				}
				else {
					node = node ? predecessor(node) : maximum(tree->root);
				}

				return *this;
			}

		private:
			friend rb_tree;

			constexpr explicit iterator_base(const rb_tree* tree, T* node) : tree {tree}, node {node} {}

			const rb_tree* tree;
			T* node;
		};

		using iterator = iterator_base<false>;
		using reverse_iterator = iterator_base<true>;

		T* get_root() const {
			return root;
		}

		static T* minimum(T* node) {
			if (!node) {
				return nullptr;
			}

			while (auto* left = (node->*Hook).get_left()) {
				node = left;
			}

			return node;
		}

		static T* maximum(T* node) {
			if (!node) {
				return nullptr;
			}

			while (auto* right = (node->*Hook).get_right()) {
				node = right;
			}

			return node;
		}

		static T* successor(T* node) {
			if (auto* right = (node->*Hook).get_right()) {
				return minimum(right);
			}

			auto* parent = (node->*Hook).get_parent();
			while (parent && (parent->*Hook).get_right() == node) {
				node = parent;
				parent = (node->*Hook).get_parent();
			}

			return parent;
		}

		static T* predecessor(T* node) {
			if (auto* left = (node->*Hook).get_left()) {
				return maximum(left);
			}

			auto* parent = (node->*Hook).get_parent();
			while (parent && (parent->*Hook).get_left() == node) {
				node = parent;
				parent = (node->*Hook).get_parent();
			}

			return parent;
		}

		T* first() const {
			return minimum(root);
		}

		T* last() const {
			return maximum(root);
		}

		template<typename K> requires requires(const T& node, const K& key) {
			node <=> key;
		}
		T* find(const K& key) const {
			T* node = root;

			while (node) {
				auto res = *node <=> key;
				if (res == 0) {
					return node;
				}

				node = (node->*Hook).children[res < 0];
			}

			return nullptr;
		}

		// Compare must return key <=> node.
		template<typename Compare> requires requires(Compare comp, const T& node) {
			comp(node);
		}
		T* find_if(Compare comp) const {
			T* node = root;

			while (node) {
				auto res = comp(*node);
				if (res == 0) {
					return node;
				}

				node = (node->*Hook).children[res > 0];
			}

			return nullptr;
		}

		template<typename K> requires requires(const T& node, const K& key) {
			node <=> key;
		}
		// Returns an iterator pointing at the first node where node.key >= key.
		// Note: The iterator is end() if there is no match.
		iterator lower_bound(const K& key) const {
			T* node = root;
			T* best = nullptr;

			while (node) {
				auto res = *node <=> key;
				if (res >= 0) {
					best = node;
					node = (node->*Hook).get_left();
				}
				else {
					node = (node->*Hook).get_right();
				}
			}

			return iterator {this, best};
		}

		template<typename K> requires requires(const T& node, const K& key) {
			node <=> key;
		}
		// Returns an iterator pointing at the first node where node.key > key.
		// Note: The iterator is end() if there is no match.
		iterator upper_bound(const K& key) const {
			T* node = root;
			T* best = nullptr;

			while (node) {
				auto res = *node <=> key;
				if (res > 0) {
					best = node;
					node = (node->*Hook).get_left();
				}
				else {
					node = (node->*Hook).get_right();
				}
			}

			return iterator {this, best};
		}

		template<typename K> requires requires(const T& node, const K& key) {
			node <=> key;
		}
		// Returns the last node where node.key <= key or nullptr if there is none.
		T* floor(const K& key) const {
			T* node = root;
			T* best = nullptr;

			while (node) {
				auto res = *node <=> key;
				// If node.key <= key then it is a candidate.
				// Continue to the larger key (right) in that case.
				if (res <= 0) {
					best = node;
					node = (node->*Hook).get_right();
				}
				else {
					node = (node->*Hook).get_left();
				}
			}

			return best;
		}

		// Note: inserting a duplicate key is undefined and crashes if HZ_ASSERTIONS is enabled.
		void insert(T* node) {
			static_assert(alignof(T) >= 2);

			(node->*Hook).children[0] = nullptr;
			(node->*Hook).children[1] = nullptr;

			if (!root) {
				(node->*Hook).set_parent_color(nullptr, false);
				root = node;

				if constexpr (requires { Augments::update(node); }) {
					Augments::update(node);
				}

				return;
			}

			T* prev = nullptr;
			T* cur = root;
			bool right = false;

			while (cur) {
				auto res = *cur <=> *node;
#if HZ_ASSERTIONS
				if (res == 0) {
#if !defined(__clang__) && defined(_MSC_VER)
				__debugbreak();
#else
				__builtin_trap();
#endif
				}
#endif

				prev = cur;
				right = res < 0;
				cur = (cur->*Hook).children[right];
			}

			(node->*Hook).set_parent_color(prev, true);
			(prev->*Hook).children[right] = node;

			if constexpr (requires { Augments::update(node); }) {
				Augments::update(node);
				for (T* p = prev; p; p = (p->*Hook).get_parent()) {
					if (!Augments::update(p)) {
						break;
					}
				}
			}

			fix_insert(node);
		}

		void remove(T* node) {
			T* start;

			if (!(node->*Hook).get_left()) {
				remove_half(node, (node->*Hook).get_right());
				start = (node->*Hook).get_parent();
			}
			else if (!(node->*Hook).get_right()) {
				remove_half(node, (node->*Hook).get_left());
				start = (node->*Hook).get_parent();
			}
			else {
				T* predec = predecessor(node);
				remove_half(predec, (predec->*Hook).get_left());
				start = (predec->*Hook).get_parent();

				if (start == node) {
					start = predec;
				}

				replace_child((node->*Hook).get_parent(), node, predec);
				(predec->*Hook).set_color((node->*Hook).is_red());

				for (int i = 0; i < 2; ++i) {
					T* child = (node->*Hook).children[i];
					(predec->*Hook).children[i] = child;

					if (child) {
						(child->*Hook).set_parent(predec);
					}
				}

				if constexpr (requires { Augments::update(start); }) {
					// Unconditionally update the nodes in the path from the old predecessor
					// to predec (the node replacement).
					while (true) {
						Augments::update(start);
						if (start == predec) {
							break;
						}

						start = (start->*Hook).get_parent();
					}
				}

				start = (start->*Hook).get_parent();
			}

			if constexpr (requires { Augments::update(start); }) {
				for (; start; start = (start->*Hook).get_parent()) {
					if (!Augments::update(start)) {
						break;
					}
				}
			}
			else {
				(void) start;
			}
		}

		[[nodiscard]] iterator begin() {
			return iterator {this, minimum(root)};
		}

		[[nodiscard]] iterator end() {
			return iterator {this, nullptr};
		}

		[[nodiscard]] reverse_iterator rbegin() {
			return reverse_iterator {this, maximum(root)};
		}

		[[nodiscard]] reverse_iterator rend() {
			return reverse_iterator {this, nullptr};
		}

	private:
		void replace_child(T* parent, T* old_child, T* new_child) {
			if (parent) {
				(parent->*Hook).children[old_child == (parent->*Hook).children[1]] = new_child;
			}
			else {
				root = new_child;
			}

			if (new_child) {
				(new_child->*Hook).set_parent(parent);
			}
		}

		void remove_half(T* node, T* child) {
			if (!(node->*Hook).is_red()) {
				if (is_red(child)) {
					(child->*Hook).set_color(false);
				}
				else {
					fix_delete(node);
				}
			}

			replace_child((node->*Hook).get_parent(), node, child);
		}

		void rotate(T* node, bool left) {
			T* new_node = (node->*Hook).children[left];

			T* new_child = (new_node->*Hook).children[!left];
			(node->*Hook).children[left] = new_child;
			if (new_child) {
				(new_child->*Hook).set_parent(node);
			}

			replace_child((node->*Hook).get_parent(), node, new_node);

			(new_node->*Hook).children[!left] = node;
			(node->*Hook).set_parent(new_node);

			if constexpr (requires { Augments::update(node); }) {
				Augments::update(node);
				Augments::update(new_node);
			}
		}

		void fix_insert(T* node) {
			auto* parent = (node->*Hook).get_parent();

			while ((parent->*Hook).is_red()) {
				T* grandparent = (parent->*Hook).get_parent();
				bool parent_is_right = parent == (grandparent->*Hook).children[1];
				T* uncle = (grandparent->*Hook).children[!parent_is_right];
				if (is_red(uncle)) {
					(parent->*Hook).set_color(false);
					(uncle->*Hook).set_color(false);

					if (grandparent == root) {
						break;
					}

					(grandparent->*Hook).set_color(true);
					node = grandparent;
					parent = (grandparent->*Hook).get_parent();
					continue;
				}

				if (node == (parent->*Hook).children[!parent_is_right]) {
					rotate(parent, !parent_is_right);
					parent = (grandparent->*Hook).children[parent_is_right];
				}

				(parent->*Hook).set_color(false);
				(grandparent->*Hook).set_color(true);
				rotate(grandparent, parent_is_right);
				break;
			}

			(root->*Hook).set_color(false);
		}

		void fix_delete(T* node) {
			T* parent = (node->*Hook).get_parent();
			while (parent) {
				bool node_is_right = node == (parent->*Hook).children[1];

				T* sibling = (parent->*Hook).children[!node_is_right];
				if ((sibling->*Hook).is_red()) {
					rotate(parent, !node_is_right);
					(parent->*Hook).set_color(true);
					(sibling->*Hook).set_color(false);
					sibling = (parent->*Hook).children[!node_is_right];
				}

				if (!is_red((sibling->*Hook).children[0]) && !is_red((sibling->*Hook).children[1])) {
					(sibling->*Hook).set_color(true);
					if (!(parent->*Hook).is_red()) {
						node = parent;
						parent = (node->*Hook).get_parent();
						continue;
					}

					(parent->*Hook).set_color(false);
					break;
				}

				bool parent_color = (parent->*Hook).is_red();
				T* sibling_child = (sibling->*Hook).children[node_is_right];
				if (is_red(sibling_child) && !is_red((sibling->*Hook).children[!node_is_right])) {
					rotate(sibling, node_is_right);
					(sibling->*Hook).set_color(true);
					(sibling_child->*Hook).set_color(false);
					sibling = sibling_child;
				}

				rotate(parent, !node_is_right);
				(parent->*Hook).set_color(false);
				(sibling->*Hook).set_color(parent_color);
				((sibling->*Hook).children[!node_is_right]->*Hook).set_color(false);
				return;
			}
		}

		static bool is_red(T* node) {
			return node && (node->*Hook).is_red();
		}

		T* root {};
	};
}
