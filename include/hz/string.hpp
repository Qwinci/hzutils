#pragma once
#include "allocator.hpp"
#include "string_view.hpp"
#include "hash.hpp"
#if __STDC_HOSTED__ == 1
#include <utility>
#else
#include "utility.hpp"
#endif

namespace hz {
	template<typename T, Allocator Allocator>
	class basic_string {
	public:
		constexpr explicit basic_string(Allocator alloc) : alloc {alloc} {
			_data = static_cast<T*>(alloc.allocate(sizeof(T)));
			_data[0] = 0;
		}

		constexpr basic_string(basic_string&& other) noexcept
			: cap {other.cap}, alloc {std::move(other.alloc)} {
			_data = other._data;
			_size = other._size;
			other._size = 0;
			other.cap = 0;
			other._data = nullptr;
		}

		constexpr basic_string(const basic_string& other) : alloc {other.alloc} {
			_data = static_cast<T*>(alloc.allocate((other._size + 1) * sizeof(T)));
			_size = other._size;
			cap = other._size;

			for (size_t i = 0; i < _size; ++i) {
				_data[i] = other._data[i];
			}
			_data[_size] = 0;
		}

		constexpr basic_string(basic_string_view<T> str, Allocator alloc) : alloc {alloc} {
			_data = static_cast<T*>(alloc.allocate((str.size() + 1) * sizeof(T)));
			_size = str.size();
			cap = str.size();

			for (size_t i = 0; i < _size; ++i) {
				_data[i] = str[i];
			}
			_data[_size] = 0;
		}

		constexpr basic_string& operator=(basic_string&& other) noexcept {
			if (_data) {
				dealloc(_data, (cap + 1) * sizeof(T));
			}

			_data = other._data;
			cap = other.cap;
			_size = other._size;
			alloc = std::move(other.alloc);

			other._size = 0;
			other.cap = 0;
			other._data = nullptr;
			return *this;
		}

		constexpr basic_string& operator=(const basic_string& other) {
			if (&other == this) {
				return *this;
			}

			if (_data) {
				dealloc(_data, (cap + 1) * sizeof(T));
			}

			alloc = other.alloc;
			_data = static_cast<T*>(alloc.allocate((other._size + 1) * sizeof(T)));
			_size = other._size;
			cap = other._size;

			for (size_t i = 0; i < _size; ++i) {
				_data[i] = other._data[i];
			}
			_data[_size] = 0;
			return *this;
		}

		constexpr basic_string& operator=(basic_string_view<T> str) {
			if (_data) {
				dealloc(_data, (cap + 1) * sizeof(T));
			}

			_data = static_cast<T*>(alloc.allocate((str.size() + 1) * sizeof(T)));
			_size = str.size();
			cap = str.size();

			for (size_t i = 0; i < _size; ++i) {
				_data[i] = str[i];
			}
			_data[_size] = 0;
			return *this;
		}

		constexpr ~basic_string() {
			if (_data) {
				dealloc(_data, (cap + 1) * sizeof(T));
			}
		}

		constexpr T& operator[](size_t index) {
			return _data[index];
		}

		constexpr const T& operator[](size_t index) const {
			return _data[index];
		}

		constexpr T& front() {
			return _data[0];
		}

		constexpr const T& front() const {
			return _data[0];
		}

		constexpr T& back() {
			return _data[_size - 1];
		}

		constexpr const T& back() const {
			return _data[_size - 1];
		}

		constexpr T* data() {
			return _data;
		}

		constexpr const T* data() const {
			return _data;
		}

		constexpr T* begin() {
			return _data;
		}

		constexpr const T* begin() const {
			return _data;
		}

		constexpr T* end() {
			return _data + _size;
		}

		constexpr const T* end() const {
			return _data + _size;
		}

		[[nodiscard]] constexpr size_t size() const {
			return _size;
		}

		[[nodiscard]] constexpr bool empty() const {
			return !_size;
		}

		void reserve(size_t new_cap) {
			if (new_cap <= cap) {
				return;
			}
			ensure_space(new_cap - cap);
		}

		[[nodiscard]] constexpr size_t capacity() const {
			return cap;
		}

		void shrink_to_fit() {
			if (cap > _size) {
				auto* new_data = static_cast<T*>(alloc.allocate((_size + 1) * sizeof(T)));

				for (size_t i = 0; i < _size; ++i) {
					new_data[i] = _data[i];
				}
				new_data[_size] = 0;

				dealloc(_data, (cap + 1) * sizeof(T));
				_data = new_data;
				cap = _size;
			}
		}

		constexpr void clear() {
			if (_size) {
				_data[0] = 0;
				_size = 0;
			}
		}

		constexpr void push_back(const T& value) {
			ensure_space(1);
			_data[_size++] = value;
			_data[_size] = 0;
		}

		constexpr void pop_back() {
			_data[--_size] = 0;
		}

		constexpr void resize(size_t count) {
			if (count <= _size) {
				_size = count;
			}
			else {
				size_t amount = count - _size;
				ensure_space(amount);
				for (size_t i = 0; i < amount; ++i) {
					_data[_size++] = 0;
				}
				_data[_size] = 0;
			}
		}

		constexpr void resize(size_t count, const T& value) {
			if (count <= _size) {
				_size = count;
			}
			else {
				size_t amount = count - _size;
				ensure_space(amount);
				for (size_t i = 0; i < amount; ++i) {
					_data[_size++] = value;
				}
				_data[_size] = 0;
			}
		}

		constexpr basic_string operator+(const basic_string& other) {
			basic_string new_str {*this};
			new_str += other;
			return new_str;
		}

		constexpr basic_string& operator+=(const basic_string& other) {
			ensure_space(other._size);
			for (size_t i = 0; i < other._size; ++i) {
				_data[_size++] = other._data[i];
			}
			_data[_size] = 0;
			return *this;
		}

		constexpr basic_string operator+(basic_string_view<T> str) {
			basic_string new_str {*this};
			new_str += str;
			return new_str;
		}

		constexpr basic_string& operator+=(basic_string_view<T> str) {
			ensure_space(str.size());
			for (size_t i = 0; i < str.size(); ++i) {
				_data[_size++] = str[i];
			}
			_data[_size] = 0;
			return *this;
		}

		constexpr basic_string operator+(T c) {
			basic_string new_str {*this};
			new_str += c;
			return new_str;
		}

		constexpr basic_string& operator+=(T c) {
			push_back(c);
			return *this;
		}

		constexpr operator basic_string_view<T>() const { // NOLINT(*-explicit-constructor)
			return {_data, _size};
		}

		constexpr basic_string_view<T> as_view() const {
			return {_data, _size};
		}

		constexpr bool operator==(const basic_string& other) const {
			return as_view() == other.as_view();
		}

		constexpr bool operator==(basic_string_view<T> str) const {
			return as_view() == str;
		}

	private:
		void ensure_space(size_t amount) {
			if (_size + amount <= cap) {
				return;
			}

			size_t new_cap = cap < 8 ? 8 : (cap + cap / 2);
			if (new_cap < cap + amount) {
				new_cap = cap + amount;
			}
			auto* new_data = static_cast<T*>(alloc.allocate((new_cap + 1) * sizeof(T)));

			for (size_t i = 0; i < _size; ++i) {
				new_data[i] = _data[i];
			}
			new_data[_size] = 0;

			dealloc(_data, (cap + 1) * sizeof(T));
			_data = new_data;
			cap = new_cap;
		}

		inline void dealloc(void* ptr, size_t size) {
			if constexpr (SizedAllocator<Allocator>) {
				alloc.deallocate(ptr, size);
			}
			else {
				alloc.deallocate(ptr);
			}
		}

		T* _data;
		size_t _size {};
		size_t cap {};
		Allocator alloc;
	};

	template<Allocator Allocator>
	using string = basic_string<char, Allocator>;

	template<typename T, typename Allocator, Hasher Hasher>
	struct hash_impl<basic_string<T, Allocator>, Hasher> {
		static void hash(Hasher& hasher, const basic_string<T, Allocator>& self) {
			for (auto c : self) {
				hasher.hash(c);
			}
		}
	};
}
