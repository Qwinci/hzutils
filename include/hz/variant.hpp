#pragma once

#include <stdint.h>

#include "config.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#include "utility.hpp"
#include "memory.hpp"
#else
#include <type_traits>
#include <utility>
#include <memory>
#endif

namespace hz {
	struct monostate {};

	namespace __detail { // NOLINT(*-reserved-identifier)
		template<typename T, typename... Types>
		struct any_of {
			static constexpr bool value = (std::is_same_v<T, Types> || ...);
		};

		template<uint8_t I, typename T, typename U, typename... Types>
		struct type_index_helper {
			static constexpr uint8_t value = std::is_same_v<T, U> ? I : type_index_helper<I + 1, T, Types...>::value;
		};
		template<uint8_t I, typename T, typename U>
		struct type_index_helper<I, T, U> {
			static constexpr uint8_t value = I;
		};

		template<typename T, typename... Types>
		struct type_index {
			static constexpr uint8_t value = type_index_helper<1, T, Types...>::value;
		};

		template<size_t I, typename U>
		constexpr auto& union_get(U& u) {
			if constexpr (I == 0) {
				return u.first;
			}
			else {
				return union_get<I - 1>(u.others);
			}
		}
	}

	template<typename... Types>
	class variant {
		static constexpr bool is_copyable = (std::is_trivially_copyable_v<Types> && ...);
		static constexpr bool is_destructible = (std::is_trivially_destructible_v<Types> && ...);

		template<typename T>
		static constexpr auto type_index = __detail::type_index<std::remove_cvref_t<T>, Types...>::value;

	public:
		static_assert(sizeof...(Types) < 255, "too many types in hz::variant");

		constexpr variant() = default;

		template<typename T> requires __detail::any_of<std::remove_cvref_t<T>, Types...>::value
		constexpr variant(T&& value) : s {std::in_place_index<type_index<T> - 1>, std::forward<T>(value)} { // NOLINT(*-explicit-constructor)
			index_ = type_index<T>;
		}

		constexpr variant(const variant& other) requires is_copyable = default;
		constexpr variant(const variant& other) : index_ {other.index_} {
			if (other.index_) {
				s.copy_from(other.index_ - 1, other.s);
			}
		}

		constexpr variant(variant&& other) noexcept requires is_copyable = default;
		constexpr variant(variant&& other) noexcept : index_ {other.index_} {
			if (other.index_) {
				s.move_from(other.index_ - 1, other.s);
			}
		}

		constexpr variant& operator=(const variant& other) requires is_copyable = default;
		constexpr variant& operator=(const variant& other) {
			reset();

			if (other.index_) {
				s.copy_from(other.index_ - 1, other.s);
				index_ = other.index_;
			}

			return *this;
		}

		constexpr variant& operator=(variant&& other) noexcept requires is_copyable = default;
		constexpr variant& operator=(variant&& other) noexcept {
			reset();

			if (other.index_) {
				s.move_from(other.index_ - 1, other.s);
				index_ = other.index_;
			}

			return *this;
		}

		template<typename T> requires __detail::any_of<T, Types...>::value
		constexpr T* get() {
			constexpr uint8_t ID = __detail::type_index<T, Types...>::value;
			if (index_ == ID) {
				return &__detail::union_get<ID - 1>(s);
			}
			else {
				return nullptr;
			}
		}

		template<typename T> requires __detail::any_of<T, Types...>::value
		constexpr const T* get() const {
			constexpr uint8_t ID = __detail::type_index<T, Types...>::value;
			if (index_ == ID) {
				return &__detail::union_get<ID - 1>(s);
			}
			else {
				return nullptr;
			}
		}

		template<typename T> requires __detail::any_of<T, Types...>::value
		constexpr T& get_unsafe() {
			constexpr uint8_t ID = __detail::type_index<T, Types...>::value;
			return __detail::union_get<ID - 1>(s);
		}

		template<typename T> requires __detail::any_of<T, Types...>::value
		constexpr const T& get_unsafe() const {
			constexpr uint8_t ID = __detail::type_index<T, Types...>::value;
			return __detail::union_get<ID - 1>(s);
		}

		[[nodiscard]] constexpr bool has_value() const {
			return index_ != 0;
		}

		[[nodiscard]] constexpr uint8_t index() const {
			return index_;
		}

		constexpr void reset() {
			if (index_) {
				if constexpr (!is_destructible) {
					s.visit(index_ - 1, [](auto& v) {
						std::destroy_at(&v);
					});
				}

				index_ = 0;
			}
		}

		constexpr ~variant() requires is_destructible = default;
		constexpr ~variant() {
			reset();
		}

	private:
		template<typename...>
		union storage {};

		template<typename T, typename... U>
		union storage<T, U...> {
			constexpr storage() {}

			template<size_t I, typename... Args>
			constexpr storage(std::in_place_index_t<I>, Args&&... args) requires (I == 0)
				: first(std::forward<Args>(args)...) {}

			template<size_t I, typename... Args>
			constexpr storage(std::in_place_index_t<I>, Args&&... args) : others {std::in_place_index<I - 1>, std::forward<Args>(args)...} {}

			constexpr ~storage() requires is_destructible = default;
			constexpr ~storage() {}

			template<typename F>
			constexpr void visit(int index, F&& fn) {
				if (index == 0) {
					fn(first);
				}
				else if constexpr (sizeof...(U) != 0) {
					others.visit(index - 1, fn);
				}
			}

			constexpr void copy_from(int index, const storage& other) {
				if (index == 0) {
					std::construct_at(&first, other.first);
				}
				else if constexpr (sizeof...(U) != 0) {
					std::construct_at(&others);
					others.copy_from(index - 1, other.others);
				}
			}

			constexpr void move_from(int index, storage& other) {
				if (index == 0) {
					std::construct_at(&first, std::move(other.first));
				}
				else if constexpr (sizeof...(U) != 0) {
					std::construct_at(&others);
					others.move_from(index - 1, other.others);
				}
			}

			T first;
			storage<U...> others;
		};

		storage<Types...> s;
		uint8_t index_ {};
	};
}
