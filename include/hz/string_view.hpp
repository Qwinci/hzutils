#pragma once
#include <stddef.h>
#include "hash.hpp"

namespace hz {
	template<typename T = char>
	constexpr size_t const_strlen(const T* str) {
		size_t len = 0;
		while (*str++) ++len;
		return len;
	}

	template<typename T>
	class basic_string_view {
	public:
		static constexpr size_t npos = -1;

		constexpr basic_string_view() : _data {}, _size {} {};

		constexpr basic_string_view(const T* str) : _data {str}, _size {const_strlen(str)} {} // NOLINT(*-explicit-constructor)

		constexpr basic_string_view(const T* str, size_t len) : _data {str}, _size {len} {}

		[[nodiscard]] constexpr size_t size() const {
			return _size;
		}

		[[nodiscard]] constexpr const T* data() const {
			return _data;
		}

		constexpr bool operator==(const basic_string_view& other) const {
			if (_size != other._size) {
				return false;
			}
			for (size_t i = 0; i < _size; ++i) {
				if (_data[i] != other._data[i]) {
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] constexpr const T& front() const {
			return _data[0];
		}

		[[nodiscard]] constexpr const T& back() const {
			return _data[_size - 1];
		}

		[[nodiscard]] constexpr const T* begin() const {
			return _data;
		}

		[[nodiscard]] constexpr const T* end() const {
			return _data + _size;
		}

		constexpr T operator[](size_t index) const {
			return _data[index];
		}

		[[nodiscard]] constexpr size_t find(T value, size_t start = 0) const {
			for (size_t i = start; i < _size; ++i) {
				if (_data[i] == value) {
					return i;
				}
			}
			return npos;
		}

		[[nodiscard]] constexpr size_t find(basic_string_view str, size_t start = 0) const {
			if (_size < str._size) {
				return npos;
			}

			for (size_t i = start; i < _size - (str._size - 1); ++i) {
				bool found = true;
				for (size_t j = 0; j < str._size; ++j) {
					if (_data[i + j] != str._data[j]) {
						found = false;
						break;
					}
				}

				if (found) {
					return i;
				}
			}

			return npos;
		}

		template<typename Comp> requires requires(Comp comp, T c) {
			static_cast<bool>(comp(c));
		}
		[[nodiscard]] constexpr size_t find(Comp comp, size_t start = 0) const {
			for (size_t i = start; i < _size; ++i) {
				if (comp(_data[i])) {
					return i;
				}
			}
			return npos;
		}

		[[nodiscard]] constexpr size_t count(T value, size_t start = 0) const {
			size_t offset = start;
			size_t count = 0;
			for (;; ++count) {
				offset = find(value, offset);
				if (offset == npos) {
					return count;
				}
				else {
					++offset;
				}
			}
		}

		[[nodiscard]] constexpr size_t count(basic_string_view str, size_t start = 0) const {
			size_t offset = start;
			size_t count = 0;
			for (;; ++count) {
				offset = find(str, offset);
				if (offset == npos) {
					return count;
				}
			}
		}

		template<typename Comp> requires requires(Comp comp, T c) {
			static_cast<bool>(comp(c));
		}
		[[nodiscard]] constexpr size_t count(Comp comp, size_t start = 0) const {
			size_t offset = start;
			size_t count = 0;
			for (;; ++count) {
				offset = find(comp, offset);
				if (offset == npos) {
					return count;
				}
			}
		}

		[[nodiscard]] constexpr size_t find_first_of(basic_string_view characters, size_t start = 0) const {
			for (size_t i = start; i < _size; ++i) {
				if (characters.find(_data[i]) != npos) {
					return i;
				}
			}
			return npos;
		}

		[[nodiscard]] constexpr size_t find_first_not_of(basic_string_view characters, size_t start = 0) const {
			for (size_t i = start; i < _size; ++i) {
				if (characters.find(_data[i]) == npos) {
					return i;
				}
			}
			return npos;
		}

		[[nodiscard]] constexpr size_t find_last_of(basic_string_view characters, size_t end = npos) const {
			if (end == npos) {
				end = _size;
			}

			for (size_t i = end; i > 0; --i) {
				if (characters.find(_data[i - 1]) != npos) {
					return i - 1;
				}
			}
			return npos;
		}

		[[nodiscard]] constexpr basic_string_view substr(size_t start = 0, size_t count = npos) const {
			if (start >= _size) {
				return {"", 0};
			}
			else if (count == npos || count > _size - start) {
				count = _size - start;
			}
			return {_data + start, count};
		}

		[[nodiscard]] constexpr basic_string_view substr_abs(size_t start = 0, size_t end = npos) const {
			size_t count;
			if (start >= _size) {
				return {"", 0};
			}
			else if (end == npos || end > _size) {
				count = _size - start;
			}
			else {
				count = end - start;
			}
			return {_data + start, count};
		}

		[[nodiscard]] constexpr bool starts_with(basic_string_view other) const {
			if (_size < other._size) {
				return false;
			}
			for (size_t i = 0; i < other._size; ++i) {
				if (_data[i] != other._data[i]) {
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] constexpr bool starts_with(T c) const {
			if (!_size) {
				return false;
			}
			return _data[0] == c;
		}

		[[nodiscard]] constexpr bool ends_with(basic_string_view other) const {
			if (_size < other._size) {
				return false;
			}
			for (size_t i = _size - other._size; i < _size; ++i) {
				if (_data[i] != other._data[i - (_size - other._size)]) {
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] constexpr bool ends_with(T c) const {
			if (!_size) {
				return false;
			}
			return _data[_size - 1] == c;
		}

		[[nodiscard]] constexpr bool contains(T c) const {
			return find(c) != npos;
		}

		[[nodiscard]] constexpr bool contains(basic_string_view str) const {
			return find(str) != npos;
		}

	protected:
		const T* _data;
		size_t _size;
	};

	template<typename T, Hasher Hasher>
	struct hash_impl<basic_string_view<T>, Hasher> {
		static void hash(Hasher& hasher, basic_string_view<T> self) {
			for (auto c : self) {
				hasher.hash(c);
			}
		}
	};

	using string_view = basic_string_view<char>;
	using wstring_view = basic_string_view<wchar_t>;

	namespace literals {
		constexpr string_view operator ""_view(const char* str) {
			return {str};
		}

#if UINTPTR_MAX == UINT64_MAX
		using __size_type = unsigned long;
#else
		using __size_type = unsigned int;
#endif

		constexpr string_view operator ""_view(const char* str, __size_type size) {
			return {str, size};
		}
	}
}
