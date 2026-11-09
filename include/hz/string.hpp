#pragma once

#include "config.hpp"

#include "vector.hpp"
#include "string_view.hpp"
#include "memory_utils.hpp"
#include "hash_impl.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	template<typename C, allocator<C> Allocator = stl_allocator<C>>
	class basic_string {
		using alloc_res = alloc_result<Allocator::infallible>;

	public:
		constexpr basic_string() = default;
		constexpr explicit basic_string(Allocator alloc) : vec {std::move(alloc)} {}

		constexpr alloc_res copy_from(basic_string_view<C> str) {
			if (auto res = vec.resize_for_overwrite(str.size() + 1); !res) {
				return res;
			}

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < str.size(); ++i) {
					std::construct_at(&vec[i], str[i]);
				}

				std::construct_at(&vec[str.size()], C {});
			}
			else {
				memory_copy(vec.data(), str.data(), str.size());
				vec[str.size()] = 0;
			}

			return alloc_res::success();
		}

		constexpr alloc_res push(C ch) requires (!Allocator::infallible) {
			if (vec.is_empty()) {
				if (auto res = vec.push(0); !res) {
					return res;
				}
			}

			if (auto res = vec.push(0); !res) {
				return res;
			}

			vec[vec.size() - 2] = ch;

			return alloc_res::success();
		}

		constexpr basic_string& push(C ch) requires Allocator::infallible {
			if (vec.is_empty()) {
				vec.push(0);
			}

			vec.push(0);
			vec[vec.size() - 2] = ch;

			return *this;
		}

		constexpr alloc_res append(basic_string_view<C> other) requires (!Allocator::infallible) {
			size_t old_size = size();
			if (auto res = vec.resize_for_overwrite(old_size + other.size() + 1); !res) {
				return res;
			}

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < other.size(); ++i) {
					std::construct_at(&vec[old_size + i], other[i]);
				}

				std::construct_at(&vec[vec.size() - 1], C {});
			}
			else {
				memory_copy(&vec[old_size], other.data(), other.size());
				vec[vec.size() - 1] = 0;
			}

			return alloc_res::success();
		}

		constexpr basic_string& append(basic_string_view<C> other) requires Allocator::infallible {
			size_t old_size = size();
			vec.resize_for_overwrite(old_size + other.size() + 1);

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < other.size(); ++i) {
					std::construct_at(&vec[old_size + i], other[i]);
				}

				std::construct_at(&vec[vec.size() - 1], C {});
			}
			else {
				memory_copy(&vec[old_size], other.data(), other.size());
				vec[vec.size() - 1] = 0;
			}

			return *this;
		}

		constexpr C pop() {
			C ch = vec[vec.size() - 2];
			vec.pop();
			vec.back() = 0;
			return ch;
		}

		constexpr alloc_res resize(size_t new_size, C ch = 0) {
			size_t old_size = size();
			if (auto res = vec.resize(new_size + 1, ch); !res) {
				return res;
			}

			if (new_size > old_size) {
				vec[old_size] = ch;
			}

			vec[new_size] = 0;
			return alloc_res::success();
		}

		// Returns a null-terminated basic_string if size() > 0.
		constexpr C* data() {
			return vec.data();
		}

		constexpr const C* data() const {
			return vec.data();
		}

		// Returns a guaranteed null-terminated basic_string.
		constexpr const C* c_str() const {
			if (vec.is_empty()) {
				if constexpr (std::is_same_v<C, char>) {
					return "";
				}
				else if constexpr (std::is_same_v<C, char8_t>) {
					return u8"";
				}
				else if constexpr (std::is_same_v<C, char16_t>) {
					return u"";
				}
				else if constexpr (std::is_same_v<C, char32_t>) {
					return U"";
				}
				else if constexpr (std::is_same_v<C, wchar_t>) {
					return L"";
				}
				else {
					static_assert(false);
				}
			}
			else {
				return vec.data();
			}
		}

		[[nodiscard]] constexpr size_t size() const {
			return vec.is_empty() ? 0 : vec.size() - 1;
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return vec.size() <= 1;
		}

		constexpr C* begin() {
			return vec.data();
		}

		constexpr const C* begin() const {
			return vec.data();
		}

		constexpr C* end() {
			return vec.data() + size();
		}

		constexpr const C* end() const {
			return vec.data() + size();
		}

		constexpr basic_string& operator+=(C ch) requires Allocator::infallible {
			return push(ch);
		}

		constexpr basic_string& operator+=(basic_string_view<C> other) requires Allocator::infallible {
			return append(other);
		}

		[[nodiscard]] constexpr basic_string_view<C> view() const {
			return {vec.data(), size()};
		}

		constexpr operator basic_string_view<C>() const {
			return view();
		}

	private:
		vector<C, Allocator> vec;
	};

	template<allocator<char> Allocator = stl_allocator<char>>
	using string = basic_string<char, Allocator>;
	template<allocator<char8_t> Allocator = stl_allocator<char8_t>>
	using u8string = basic_string<char8_t, Allocator>;
	template<allocator<char16_t> Allocator = stl_allocator<char16_t>>
	using u16string = basic_string<char16_t, Allocator>;
	template<allocator<char32_t> Allocator = stl_allocator<char32_t>>
	using u32string = basic_string<char32_t, Allocator>;
	template<allocator<wchar_t> Allocator = stl_allocator<wchar_t>>
	using wstring = basic_string<wchar_t, Allocator>;

	template<typename C, allocator<C> Allocator, hasher Hasher>
	struct hash_impl<basic_string<C, Allocator>, Hasher> {
		static constexpr void hash(const basic_string<C, Allocator>& self, Hasher& hasher) {
			hash_impl<basic_string_view<C>, Hasher>::hash(self, hasher);
		}
	};
}
