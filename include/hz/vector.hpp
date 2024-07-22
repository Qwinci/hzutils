#pragma once
#include "allocator.hpp"
#if __STDC_HOSTED__ == 1
#include <new>
#include <utility>
#else
#include "new.hpp"
#include "utility.hpp"
#endif

namespace hz {
	template<typename T, Allocator Allocator>
	class vector {
	public:
		constexpr explicit vector(Allocator alloc) : alloc {alloc} {}

		constexpr vector(vector&& other) noexcept
			: _data {other._data}, cap {other.cap}, _size {other._size},
				alloc {std::move(other.alloc)} {
			other._size = 0;
			other.cap = 0;
			other._data = nullptr;
		}

		constexpr vector(const vector& other) : alloc {other.alloc} {
			if (other._size) {
				_data = static_cast<T*>(alloc.allocate(other._size * sizeof(T)));
				_size = other._size;
				cap = other._size;

				for (size_t i = 0; i < _size; ++i) {
					new (&_data[i]) T {other._data[i]};
				}
			}
		}

		constexpr vector& operator=(vector&& other) noexcept {
			if (cap) {
				for (size_t i = 0; i < _size; ++i) {
					_data[i].~T();
				}
				dealloc(_data, cap * sizeof(T));
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

		constexpr vector& operator=(const vector& other) {
			if (&other == this) {
				return *this;
			}

			if (cap) {
				for (size_t i = 0; i < _size; ++i) {
					_data[i].~T();
				}
				dealloc(_data, cap * sizeof(T));
			}

			alloc = other.alloc;
			_data = static_cast<T*>(alloc.allocate(other._size * sizeof(T)));
			_size = other._size;
			cap = other._size;

			for (size_t i = 0; i < _size; ++i) {
				new (&_data[i]) T {other._data[i]};
			}
			return *this;
		}

		constexpr ~vector() {
			if (cap) {
				for (size_t i = 0; i < _size; ++i) {
					_data[i].~T();
				}
				dealloc(_data, cap * sizeof(T));
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

		[[nodiscard]] constexpr bool empty() const {
			return !_size;
		}

		[[nodiscard]] constexpr size_t size() const {
			return _size;
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
				T* new_data = nullptr;
				if (_size) {
					new_data = static_cast<T*>(alloc.allocate(_size * sizeof(T)));
					for (size_t i = 0; i < _size; ++i) {
						new (&new_data[i]) T {std::move(_data[i])};
						_data[i].~T();
					}
				}

				dealloc(_data, cap * sizeof(T));
				_data = new_data;
				cap = _size;
			}
		}

		constexpr void clear() {
			if (_size) {
				for (size_t i = 0; i < _size; ++i) {
					_data[i].~T();
				}
				_size = 0;
			}
		}

		constexpr void push_back(const T& value) {
			ensure_space(1);
			new (&_data[_size++]) T {value};
		}

		constexpr void push_back(T&& value) {
			ensure_space(1);
			new (&_data[_size++]) T {std::move(value)};
		}

		template<typename... Args>
		constexpr T& emplace_back(Args&&... args) {
			ensure_space(1);
			return *new (&_data[_size++]) T {std::forward<Args&&>(args)...};
		}

		constexpr void pop_back() {
			_data[--_size].~T();
		}

		constexpr void resize(size_t count) {
			if (count <= _size) {
				for (size_t i = count; i < _size; ++i) {
					_data[i].~T();
				}
				_size = count;
			}
			else {
				size_t amount = count - _size;
				ensure_space(amount);
				for (size_t i = 0; i < amount; ++i) {
					new (&_data[_size++]) T {};
				}
			}
		}

		constexpr void resize(size_t count, const T& value) {
			if (count <= _size) {
				for (size_t i = count; i < _size; ++i) {
					_data[i].~T();
				}
				_size = count;
			}
			else {
				size_t amount = count - _size;
				ensure_space(amount);
				for (size_t i = 0; i < amount; ++i) {
					new (&_data[_size++]) T {value};
				}
			}
		}

		constexpr vector new_with_alloc() const {
			return vector {alloc};
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
			auto* new_data = static_cast<T*>(alloc.allocate(new_cap * sizeof(T)));

			for (size_t i = 0; i < _size; ++i) {
				new (&new_data[i]) T {std::move(_data[i])};
				_data[i].~T();
			}

			dealloc(_data, cap * sizeof(T));
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

		T* _data {};
		size_t cap {};
		size_t _size {};
		Allocator alloc;
	};
}
