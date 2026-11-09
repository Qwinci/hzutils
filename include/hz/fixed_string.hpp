#pragma once

#include "config.hpp"

#include "fixed_vector.hpp"
#include "string_view.hpp"
#include "memory_utils.hpp"
#include "hash_impl.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

namespace hz {
	template<typename C, size_t N>
	class basic_fixed_string {
	public:
		constexpr basic_fixed_string() : vec(1) {}
		constexpr basic_fixed_string(basic_string_view<C> str) {
			vec.resize_for_overwrite(str.size() + 1);

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < str.size(); ++i) {
					vec[i] = str[i];
				}
			}
			else {
				memory_copy(vec.data(), str.data(), str.size());
			}

			vec[str.size()] = 0;
		}

		constexpr basic_fixed_string& push(C ch) {
			size_t old_size = vec.size();
			vec.resize_for_overwrite(old_size + 1);

			vec[old_size - 1] = ch;
			vec[old_size] = 0;

			return *this;
		}

		constexpr basic_fixed_string& append(basic_string_view<C> other) {
			size_t old_size = vec.size();
			vec.resize_for_overwrite(old_size + other.size());

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < other.size(); ++i) {
					vec[old_size - 1 + i] = other[i];
				}
			}
			else {
				memory_copy(&vec[old_size - 1], other.data(), other.size());
			}

			vec[vec.size() - 1] = 0;

			return *this;
		}

		constexpr C pop() {
			return vec.pop();
		}

		constexpr void resize(size_t new_size, C ch = 0) {
			vec.resize(new_size, ch);
		}

		constexpr C* data() {
			return vec.data();
		}

		constexpr const C* data() const {
			return vec.data();
		}

		[[nodiscard]] constexpr size_t size() const {
			return vec.size() - 1;
		}

		[[nodiscard]] constexpr size_t is_empty() const {
			return vec.size() <= 1;
		}

		constexpr C* begin() {
			return vec.data();
		}

		constexpr const C* begin() const {
			return vec.data();
		}

		constexpr C* end() {
			return vec.end() - 1;
		}

		constexpr const C* end() const {
			return vec.end() - 1;
		}

		constexpr basic_fixed_string& operator+=(C ch) {
			return push(ch);
		}

		constexpr basic_fixed_string& operator+=(basic_string_view<C> other) {
			return append(other);
		}

		[[nodiscard]] constexpr basic_string_view<C> view() const {
			return {vec.data(), vec.size() - 1};
		}

		constexpr operator basic_string_view<C>() const {
			return view();
		}

	private:
		fixed_vector<C, N> vec;
	};

	template<size_t N>
	using fixed_string = basic_fixed_string<char, N>;
	template<size_t N>
	using fixed_u8string = basic_fixed_string<char8_t, N>;
	template<size_t N>
	using fixed_u16string = basic_fixed_string<char16_t, N>;
	template<size_t N>
	using fixed_u32string = basic_fixed_string<char32_t, N>;
	template<size_t N>
	using fixed_wstring = basic_fixed_string<wchar_t, N>;

	template<typename C, size_t N, hasher Hasher>
	struct hash_impl<basic_fixed_string<C, N>, Hasher> {
		static constexpr void hash(const basic_fixed_string<C, N>& self, Hasher& hasher) {
			hash_impl<basic_string_view<C>, Hasher>::hash(self, hasher);
		}
	};
}
