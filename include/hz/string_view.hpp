#pragma once

#include <stddef.h>

#include "config.hpp"
#include "string_utils.hpp"
#include "hash_impl.hpp"

#if HZ_FREESTANDING
#include "compare.hpp"
#include "type_traits.hpp"
#else
#include <compare>
#include <type_traits>
#endif

namespace hz {
	inline constexpr size_t npos = static_cast<size_t>(-1);

	template<typename T>
	class codepoint_iterator;

	template<typename It>
	class utf8_encode_iterator;

	template<typename It>
	class utf16_encode_iterator;

	template<typename T>
	class basic_string_view {
	public:
		constexpr basic_string_view() = default;
		constexpr basic_string_view(const T* str) : data_ {str}, size_ {const_strlen(str)} {}
		constexpr basic_string_view(const T* data, size_t size) : data_ {data}, size_ {size} {}

		constexpr bool operator==(const basic_string_view& other) const {
			if (size_ != other.size_) {
				return false;
			}
			else if (!size_) {
				return true;
			}

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < size_; ++i) {
					if (data_[i] != other.data_[i]) {
						return false;
					}
				}

				return true;
			}

			return __builtin_memcmp(data_, other.data_, size_ * sizeof(T)) == 0;
		}

		constexpr std::strong_ordering operator<=>(const basic_string_view& other) const {
			size_t min_size = size_ < other.size_ ? size_ : other.size_;

			if constexpr (sizeof(T) == 1) {
				if (std::is_constant_evaluated()) {
					for (size_t i = 0; i < min_size; ++i) {
						int res = static_cast<unsigned char>(data_[i]) - static_cast<unsigned char>(other.data_[i]);
						if (res < 0) {
							return std::strong_ordering::less;
						}
						if (res > 0) {
							return std::strong_ordering::greater;
						}
					}
				}
				else {
					int res = __builtin_memcmp(data_, other.data_, min_size);
					if (res < 0) {
						return std::strong_ordering::less;
					}
					else if (res > 0) {
						return std::strong_ordering::greater;
					}
				}
			}
			else {
				for (size_t i = 0; i < min_size; ++i) {
					if (data_[i] != other.data_[i]) {
						return data_[i] < other.data_[i] ? std::strong_ordering::less : std::strong_ordering::greater;
					}
				}
			}

			return size_ == other.size_ ? std::strong_ordering::equal :
				size_ < other.size_ ? std::strong_ordering::less : std::strong_ordering::greater;
		}

		[[nodiscard]] constexpr std::strong_ordering shortlex_cmp(const basic_string_view& other) const {
			if (size_ < other.size_) {
				return std::strong_ordering::less;
			}
			if (size_ > other.size_) {
				return std::strong_ordering::greater;
			}

			if constexpr (sizeof(T) == 1) {
				if (std::is_constant_evaluated()) {
					for (size_t i = 0; i < size_; ++i) {
						int res = static_cast<unsigned char>(data_[i]) - static_cast<unsigned char>(other.data_[i]);
						if (res < 0) {
							return std::strong_ordering::less;
						}
						if (res > 0) {
							return std::strong_ordering::greater;
						}
					}

					return std::strong_ordering::equal;
				}

				int res = __builtin_memcmp(data_, other.data_, size_);
				return res < 0 ? std::strong_ordering::less :
					(res > 0 ? std::strong_ordering::greater : std::strong_ordering::equal);
			}
			else {
				for (size_t i = 0; i < size_; ++i) {
					if (data_[i] != other.data_[i]) {
						return data_[i] < other.data_[i] ? std::strong_ordering::less : std::strong_ordering::greater;
					}
				}

				return std::strong_ordering::equal;
			}
		}

		[[nodiscard]] constexpr bool case_insensitive_compare(basic_string_view other) const {
			if (size_ != other.size_) {
				return false;
			}

			for (size_t i = 0; i < size_; ++i) {
				if (to_lower_ascii(data_[i]) != to_lower_ascii(other.data_[i])) {
					return false;
				}
			}

			return true;
		}

		[[nodiscard]] constexpr size_t find(T ch, size_t start = 0) const {
			for (size_t i = start; i < size_; ++i) {
				if (data_[i] == ch) {
					return i;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_t find(basic_string_view other, size_t start = 0) const {
			if (size_ < other.size_) {
				return npos;
			}

			for (size_t i = start; i <= size_ - other.size_; ++i) {
				if (std::is_constant_evaluated()) {
					bool found = true;

					for (size_t j = 0; j < other.size_; ++j) {
						if (data_[i + j] != other.data_[j]) {
							found = false;
							break;
						}
					}

					if (found) {
						return i;
					}

					continue;
				}

				if (__builtin_memcmp(data_ + i, other.data_, other.size_ * sizeof(T)) == 0) {
					return i;
				}
			}

			return npos;
		}

		template<typename Compare> requires requires(Compare comp, T ch) {
			static_cast<bool>(comp(ch));
		}
		[[nodiscard]] constexpr size_t find(Compare comp, size_t start = 0) const {
			for (size_t i = start; i < size_; ++i) {
				if (comp(data_[i])) {
					return i;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_t rfind(T ch, size_t end = npos) const {
			if (!size_) {
				return npos;
			}
			else if (end >= size_) {
				end = size_ - 1;
			}

			for (size_t i = end + 1; i > 0; --i) {
				if (data_[i - 1] == ch) {
					return i - 1;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_t rfind(basic_string_view other, size_t end = npos) const {
			if (!other.size_) {
				return 0;
			}

			if (!size_) {
				return npos;
			}
			else if (end >= size_) {
				end = size_ - 1;
			}

			if (end + 1 < other.size_) {
				return npos;
			}

			for (size_t i = end + 1 - (other.size_ - 1); i > 0; --i) {
				if (std::is_constant_evaluated()) {
					bool found = true;

					for (size_t j = 0; j < other.size_; ++j) {
						if (data_[i - 1 + j] != other.data_[j]) {
							found = false;
							break;
						}
					}

					if (found) {
						return i - 1;
					}

					continue;
				}

				if (__builtin_memcmp(data_ + i - 1, other.data_, other.size_ * sizeof(T)) == 0) {
					return i - 1;
				}
			}

			return npos;
		}

		template<typename Compare> requires requires(Compare comp, T ch) {
			static_cast<bool>(comp(ch));
		}
		[[nodiscard]] constexpr size_t rfind(Compare comp, size_t end = npos) const {
			if (!size_) {
				return npos;
			}
			else if (end >= size_) {
				end = size_ - 1;
			}

			for (size_t i = end + 1; i > 0; --i) {
				if (comp(data_[i - 1])) {
					return i - 1;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_t find_first_of(basic_string_view chars, size_t start = 0) const {
			for (size_t i = start; i < size_; ++i) {
				if (chars.find(data_[i]) != npos) {
					return i;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_t find_first_not_of(basic_string_view chars, size_t start = 0) const {
			for (size_t i = start; i < size_; ++i) {
				if (chars.find(data_[i]) == npos) {
					return i;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_t find_last_of(basic_string_view chars, size_t end = npos) const {
			if (!size_) {
				return npos;
			}
			else if (end >= size_) {
				end = size_ - 1;
			}

			for (size_t i = end + 1; i > 0; --i) {
				if (chars.find(data_[i - 1]) != npos) {
					return i - 1;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_t find_last_not_of(basic_string_view chars, size_t end = npos) const {
			if (!size_) {
				return npos;
			}
			else if (end >= size_) {
				end = size_ - 1;
			}

			for (size_t i = end + 1; i > 0; --i) {
				if (chars.find(data_[i - 1]) == npos) {
					return i - 1;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr basic_string_view substr(size_t start = 0, size_t count = npos) const {
			if (start >= size_) {
				return {data_ + size_, 0};
			}
			else if (count > size_ - start) {
				count = size_ - start;
			}

			return {data_ + start, count};
		}

		[[nodiscard]] constexpr basic_string_view substr_abs(size_t start = 0, size_t end = npos) const {
			size_t count;
			if (start >= size_) {
				return {data_ + size_, 0};
			}
			else if (end > size_) {
				count = size_ - start;
			}
			else {
				count = end - start;
			}

			return {data_ + start, count};
		}

		[[nodiscard]] constexpr bool starts_with(T ch) const {
			return size_ && data_[0] == ch;
		}

		[[nodiscard]] constexpr bool starts_with(basic_string_view other) const {
			if (size_ < other.size_) {
				return false;
			}
			if (other.size_ == 0) {
				return true;
			}

			if (std::is_constant_evaluated()) {
				for (size_t i = 0; i < other.size_; ++i) {
					if (data_[i] != other.data_[i]) {
						return false;
					}
				}

				return true;
			}

			return __builtin_memcmp(data_, other.data_, other.size_ * sizeof(T)) == 0;
		}

		[[nodiscard]] constexpr bool ends_with(T ch) const {
			return size_ && data_[size_ - 1] == ch;
		}

		[[nodiscard]] constexpr bool ends_with(basic_string_view other) const {
			if (size_ < other.size_) {
				return false;
			}
			if (other.size_ == 0) {
				return true;
			}

			if (std::is_constant_evaluated()) {
				for (size_t i = size_ - other.size_; i < size_; ++i) {
					if (data_[i] != other.data_[i - (size_ - other.size_)]) {
						return false;
					}
				}

				return true;
			}

			return __builtin_memcmp(data_ + (size_ - other.size_), other.data_, other.size_ * sizeof(T)) == 0;
		}

		[[nodiscard]] constexpr bool contains(T ch) const {
			return find(ch) != npos;
		}

		[[nodiscard]] constexpr bool contains(basic_string_view other) const {
			return find(other) != npos;
		}

		[[nodiscard]] constexpr const T* data() const {
			return data_;
		}

		[[nodiscard]] constexpr size_t size() const {
			return size_;
		}

		[[nodiscard]] constexpr bool is_empty() const {
			return !size_;
		}

		constexpr const T& operator[](size_t index) const {
			return data_[index];
		}

		[[nodiscard]] constexpr const T& front() const {
			return data_[0];
		}

		[[nodiscard]] constexpr const T& back() const {
			return data_[size_ - 1];
		}

		[[nodiscard]] constexpr const T* begin() const {
			return data_;
		}

		[[nodiscard]] constexpr const T* end() const {
			return data_ + size_;
		}

		constexpr bool remove_prefix(basic_string_view prefix) {
			if (starts_with(prefix)) {
				data_ += prefix.size_;
				size_ -= prefix.size_;
				return true;
			}
			else {
				return false;
			}
		}

		constexpr bool remove_suffix(basic_string_view suffix) {
			if (ends_with(suffix)) {
				size_ -= suffix.size_;
				return true;
			}
			else {
				return false;
			}
		}

		constexpr basic_string_view trim_start(basic_string_view chars) {
			const T* start_ptr = data_;
			const T* end_ptr = data_ + size_;

			while (start_ptr != end_ptr && chars.contains(*start_ptr)) {
				++start_ptr;
			}

			return {start_ptr, static_cast<size_t>(end_ptr - start_ptr)};
		}

		constexpr basic_string_view trim_end(basic_string_view chars) {
			const T* start_ptr = data_;
			const T* end_ptr = data_ + size_;

			while (end_ptr != start_ptr && chars.contains(*(end_ptr - 1))) {
				--end_ptr;
			}

			return {start_ptr, static_cast<size_t>(end_ptr - start_ptr)};
		}

		constexpr basic_string_view trim(basic_string_view chars) {
			const T* start_ptr = data_;
			const T* end_ptr = data_ + size_;

			while (start_ptr != end_ptr && chars.contains(*start_ptr)) {
				++start_ptr;
			}

			while (end_ptr != start_ptr && chars.contains(*(end_ptr - 1))) {
				--end_ptr;
			}

			return {start_ptr, static_cast<size_t>(end_ptr - start_ptr)};
		}

		class split_iterator {
		public:
			class inner {
			public:
				constexpr basic_string_view operator*() const {
					return str.substr_abs(start_pos, end_pos);
				}

				constexpr bool operator==(int) const {
					return start_pos == npos;
				}

				constexpr inner& operator++() {
					if (end_pos == npos) {
						start_pos = npos;
						return *this;
					}

					start_pos = end_pos + 1;
					end_pos = str.find_first_of(delimiters, start_pos);
					return *this;
				}

			private:
				friend split_iterator;

				constexpr inner(basic_string_view str, basic_string_view delimiters)
					:  str {str}, delimiters {delimiters} {
					end_pos = str.find_first_of(delimiters);
				}

				basic_string_view str;
				basic_string_view delimiters;
				size_t start_pos {};
				size_t end_pos {};
			};

			constexpr inner begin() const {
				return inner {str, delimiters};
			}

			[[nodiscard]] constexpr int end() const {
				return 0;
			}

		private:
			friend basic_string_view;

			constexpr split_iterator(basic_string_view str, basic_string_view delimiters)
				: str {str}, delimiters {delimiters} {}

			basic_string_view str;
			basic_string_view delimiters;
		};

		[[nodiscard]] constexpr split_iterator split(basic_string_view delimiters) const {
			return split_iterator {*this, delimiters};
		}

		[[nodiscard]] constexpr codepoint_iterator<T> codepoints() const;
		[[nodiscard]] constexpr auto utf8_encode() const;
		[[nodiscard]] constexpr auto utf16_encode() const;
		[[nodiscard]] constexpr codepoint_iterator<T> utf32_encode() const;

	private:
		const T* data_ {};
		size_t size_ {};
	};

	template<typename T, hasher Hasher>
	struct hash_impl<basic_string_view<T>, Hasher> {
		static constexpr void hash(const basic_string_view<T>& self, Hasher& hasher) {
			hasher.hash_bytes(self.data(), self.size());
			hasher.hash(self.size());
		}
	};

	using string_view = basic_string_view<char>;
	using wstring_view = basic_string_view<wchar_t>;
	using u8string_view = basic_string_view<char8_t>;
	using u16string_view = basic_string_view<char16_t>;
	using u32string_view = basic_string_view<char32_t>;

	namespace literals {
		constexpr string_view operator""_sv(const char* str, size_t size) {
			return {str, size};
		}

		constexpr wstring_view operator""_sv(const wchar_t* str, size_t size) {
			return {str, size};
		}

		constexpr u8string_view operator""_sv(const char8_t* str, size_t size) {
			return {str, size};
		}

		constexpr u16string_view operator""_sv(const char16_t* str, size_t size) {
			return {str, size};
		}

		constexpr u32string_view operator""_sv(const char32_t* str, size_t size) {
			return {str, size};
		}
	}
}
