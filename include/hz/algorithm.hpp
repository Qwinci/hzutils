#pragma once

#include "config.hpp"
#include "memory_utils.hpp"

#include <stddef.h>

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

#if HZ_FREESTANDING
#include "utility.hpp"
#include "type_traits.hpp"
#else
#include <utility>
#include <type_traits>
#endif

namespace hz {
	template<typename It, typename T>
	constexpr It find(It first, It last, const T& value) {
		for (auto it = first; it != last; ++it) {
			if (*it == value) {
				return it;
			}
		}

		return last;
	}

	template<typename It, typename Compare>
	constexpr It find_if(It first, It last, Compare comp) {
		for (auto it = first; it != last; ++it) {
			if (comp(*it)) {
				return it;
			}
		}

		return last;
	}

	template<typename It, typename Compare>
	constexpr It find_if_not(It first, It last, Compare comp) {
		for (auto it = first; it != last; ++it) {
			if (!comp(*it)) {
				return it;
			}
		}

		return last;
	}

	// Returns an iterator pointing at the first element > key.
	// Note: The iterator is last if there is no match.
	template<typename It, typename T>
	constexpr It upper_bound(It first, It last, const T& value) {
		while (first != last) {
			It mid = first + (last - first) / 2;

			if (*mid <= value) {
				first = mid + 1;
			}
			else {
				last = mid;
			}
		}

		return first;
	}

	// Returns an iterator pointing at the first element >= key.
	// Note: The iterator is last if there is no match.
	template<typename It, typename T>
	constexpr It lower_bound(It first, It last, const T& value) {
		while (first != last) {
			It mid = first + (last - first) / 2;
			if (*mid < value) {
				first = mid + 1;
			}
			else {
				last = mid;
			}
		}

		return first;
	}

	template<typename It, typename T>
	constexpr It find_sorted(It first, It last, const T& value) {
		auto it = hz::lower_bound(first, last, value);
		return it != last && *it == value ? it : last;
	}

	template<typename It>
	constexpr It min_element(It first, It last) {
		It min = first;
		for (; first != last; ++first) {
			if (*first < *min) {
				min = first;
			}
		}

		return min;
	}

	template<typename It>
	constexpr It max_element(It first, It last) {
		It max = first;
		for (; first != last; ++first) {
			if (*max < *first) {
				max = first;
			}
		}

		return max;
	}

	template<typename It, typename Predicate>
	constexpr It remove_if(It first, It last, Predicate pred) {
		auto new_it = first;

		for (; first != last; ++first) {
			if (pred(*first)) {
				continue;
			}

			if (new_it != first) {
				*new_it = std::move(*first);
			}
			++new_it;
		}

		return new_it;
	}

	template<typename It>
	constexpr void reverse(It first, It last) {
		using std::swap;

		for (; first != last && first != --last; ++first) {
			swap(*first, *last);
		}
	}

	template<typename It, typename Predicate>
	constexpr bool any_of(It first, It last, Predicate pred) {
		for (; first != last; ++first) {
			if (pred(*first)) {
				return true;
			}
		}

		return false;
	}

	template<typename It, typename Predicate>
	constexpr bool all_of(It first, It last, Predicate pred) {
		for (; first != last; ++first) {
			if (!pred(*first)) {
				return false;
			}
		}

		return true;
	}

	template<typename It1, typename It2>
	constexpr It2 move_backward(It1 first, It1 last, It2 dest_last) {
		while (first != last) {
			*--dest_last = std::move(*--last);
		}

		return dest_last;
	}

	// Copies elements from the source range to the destination range.
	// Note: If the element type is trivially copyable the destination
	// must not overlap with the source range.
	// Returns the destination iterator pointing after the copied elements.
	template<typename It1, typename It2>
	constexpr It2 copy(It1 first, It1 last, It2 dest_first) {
		using T = std::remove_cvref_t<decltype(*first)>;

		if constexpr (std::is_pointer_v<It1> && std::is_pointer_v<It2> &&
			std::is_same_v<T, std::remove_cvref_t<decltype(*dest_first)>> &&
			std::is_trivially_copyable_v<T>) {

			if (!std::is_constant_evaluated()) {
				size_t count = static_cast<size_t>(last - first);
				if (count) {
					memory_copy(dest_first, first, count);
				}

				return dest_first + count;
			}
		}

		for (; first != last; ++first, ++dest_first) {
			*dest_first = *first;
		}

		return dest_first;
	}

	template<typename It, typename T>
	constexpr void fill(It first, It last, const T& value) {
		using U = std::remove_cvref_t<decltype(*first)>;

		if constexpr (std::is_pointer_v<It> && sizeof(U) == 1 && std::is_integral_v<U>) {
			if (!std::is_constant_evaluated()) {
				auto count = static_cast<size_t>(last - first);
				if (count) {
					memory_set(first, static_cast<unsigned char>(static_cast<U>(value)), count);
					return;
				}
			}
		}

		for (; first != last; ++first) {
			*first = value;
		}
	}

	namespace __detail {
		template<typename It>
		constexpr It quicksort_partition(It first, It last) {
			using std::swap;

			It pivot_it = first + (last - first - 1) / 2;

			--last;

			while (true) {
				while (*first < *pivot_it) ++first;
				while (*pivot_it < *last) --last;

				if (first >= last) {
					return last;
				}

				swap(*first, *last);

				if (first == pivot_it) {
					pivot_it = last;
				}
				else if (last == pivot_it) {
					pivot_it = first;
				}

				++first;
				--last;
			}
		}

		template<typename It>
		constexpr void quicksort(It first, It last) {
			if (first + 1 < last) {
				It p = quicksort_partition(first, last);
				++p;
				quicksort(first, p);
				quicksort(p, last);
			}
		}

		template<typename It>
		constexpr void insertion_sort(It first, It last) {
			for (auto it = first + 1; it < last; ++it) {
				auto elem = std::move(*it);

				if (elem < *first) {
					hz::move_backward(first, it, it + 1);
					*first = std::move(elem);
				}
				else {
					auto it2 = it;
					for (; elem < *(it2 - 1); --it2) {
						*it2 = std::move(*(it2 - 1));
					}
					*it2 = std::move(elem);
				}
			}
		}

		template<typename It>
		constexpr void heapsort(It first, It last) {
			using std::swap;

			auto parent = [](ptrdiff_t i) {
				return (i - 1) / 2;
			};

			auto leaf_search = [=](ptrdiff_t i, ptrdiff_t end) {
				auto left_child = [](ptrdiff_t i) {
					return i * 2 + 1;
				};
				auto right_child = [](ptrdiff_t i) {
					return i * 2 + 2;
				};

				ptrdiff_t j = i;
				for (auto right = right_child(j); right < end; right = right_child(j)) {
					auto left = left_child(j);
					if (*(first + right) < *(first + left)) {
						j = left;
					}
					else {
						j = right;
					}
				}

				auto left = left_child(j);
				if (left < end) {
					j = left;
				}

				return j;
			};

			ptrdiff_t start = (last - first) / 2;
			ptrdiff_t end = last - first;
			while (end > 1) {
				if (start > 0) {
					--start;
				}
				else {
					--end;
					swap(*(first + end), *first);
				}

				ptrdiff_t j = leaf_search(start, end);
				while (*(first + j) < *(first + start)) {
					j = parent(j);
				}

				for (; j > start; j = parent(j)) {
					swap(*(first + start), *(first + j));
				}
			}
		}

		template<typename It>
		constexpr void introsort(It first, It last, ptrdiff_t depth) {
			if (last - first < 16) {
				insertion_sort(first, last);
			}
			else if (depth == 0) {
				heapsort(first, last);
			}
			else {
				It p = quicksort_partition(first, last);
				++p;
				introsort(first, p, depth - 1);
				introsort(p, last, depth - 1);
			}
		}

		template<typename T>
		constexpr int log2(T value) noexcept {
			constexpr int bits = static_cast<int>(sizeof(T)) * 8;

#if defined(_MSC_VER) && !defined(__clang__)
			if (std::is_constant_evaluated()) {
				for (int i = bits - 1; i >= 0; --i) {
					if (value & (1LL << i)) {
						return i + 1;
					}
				}

				__assume(false);
			}
#endif

			if constexpr (sizeof(T) == 4) {
#if defined(_MSC_VER) && !defined(__clang__)
				unsigned long index;
				_BitScanReverse(&index, static_cast<unsigned long>(value));
				return index + 1;
#else
				return bits - __builtin_clz(value);
#endif
			}
			else {
#if defined(_MSC_VER) && !defined(__clang__)
				auto v = static_cast<unsigned long long>(value);
				unsigned long index;
				_BitScanReverse64(&index, v);
				return index + 1;
#else
				return bits - __builtin_clzll(value);
#endif
			}
		}
	}

	template<typename It>
	constexpr void sort(It first, It last) {
		if (first == last) {
			return;
		}

		__detail::introsort(first, last, __detail::log2(last - first) * 2);
	}
}
