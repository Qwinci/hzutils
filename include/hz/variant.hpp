#pragma once
#include "algorithm.hpp"
#include "type_traits.hpp"
#if __STDC_HOSTED__ == 1
#include <utility>
#include <new>
#else
#include "utility.hpp"
#include "new.hpp"
#endif

namespace hz {
	namespace __detail {
		template<typename T, typename... Types>
		struct is_any_of_helper {
			static constexpr bool value = (is_same_v<T, Types> || ...);
		};

		template<size_t I, typename Expected, typename T, typename... Types>
		struct index_of_helper {
			static constexpr size_t value = is_same_v<T, Expected> ? I : index_of_helper<I + 1, Expected, Types...>::value;
		};
		template<size_t I, typename Expected, typename T>
		struct index_of_helper<I, Expected, T> {
			static constexpr size_t value = I;
		};

		template<typename T, typename... Types>
		struct index_of {
			static constexpr size_t value = index_of_helper<0, T, Types...>::value;
		};
	}

	struct monostate {};

	template<typename... Types>
	class variant {
	private:
		static constexpr bool is_copyable = (is_trivially_copyable_v<Types> && ...);

	public:
		constexpr variant() = default;
		template<typename T> requires(__detail::is_any_of_helper<remove_reference_t<T>, Types...>::value)
		variant(T&& value) { // NOLINT(*-explicit-constructor)
			constexpr size_t ID = __detail::index_of<remove_reference_t<T>, Types...>::value;
			id = ID;
			new (&storage) remove_reference_t<T> {std::forward<T>(value)};
		}

		variant(variant&& other) requires(is_copyable) = default;
		variant(variant&& other) noexcept requires(!is_copyable) {
			if (other.id == SIZE_MAX) {
				id = SIZE_MAX;
				return;
			}
			(move_from<Types>(other) || ...);
		}

		variant(const variant& other) requires(is_copyable) = default;
		variant(const variant& other) requires(!is_copyable) {
			if (other.id == SIZE_MAX) {
				id = SIZE_MAX;
				return;
			}
			(copy_from<Types>(other) || ...);
		}

		variant& operator=(variant&& other) requires(is_copyable) = default;
		variant& operator=(variant&& other) noexcept requires(!is_copyable) {
			if (id != SIZE_MAX) {
				(destroy<Types>() || ...);
			}

			if (other.id == SIZE_MAX) {
				id = SIZE_MAX;
			}
			else {
				(move_from<Types>(other) || ...);
			}
			return *this;
		}
		variant& operator=(const variant& other) requires(is_copyable) = default;
		variant& operator=(const variant& other) requires(!is_copyable) {
			if (&other == this) {
				return *this;
			}

			if (id != SIZE_MAX) {
				(destroy<Types>() || ...);
			}
			if (other.id == SIZE_MAX) {
				id = SIZE_MAX;
			}
			else {
				(copy_from<Types>(other) || ...);
			}

			return *this;
		}

		template<typename T> requires(__detail::is_any_of_helper<T, Types...>::value)
		[[nodiscard]] const T* get() const {
			constexpr size_t ID = __detail::index_of<T, Types...>::value;
			if (id == ID) {
				return std::launder(reinterpret_cast<T*>(&storage));
			}
			else {
				return nullptr;
			}
		}

		template<typename T> requires(__detail::is_any_of_helper<T, Types...>::value)
		[[nodiscard]] T* get() {
			constexpr size_t ID = __detail::index_of<T, Types...>::value;
			if (id == ID) {
				return std::launder(reinterpret_cast<T*>(&storage));
			}
			else {
				return nullptr;
			}
		}

		template<typename Visitor>
		void visit(Visitor&& visitor) {
			(visit_internal<Visitor, Types>(visitor) || ...);
		}

		template<typename T> requires(__detail::is_any_of_helper<T, Types...>::value)
		[[nodiscard]] const T& get_unsafe() const {
			return *std::launder(reinterpret_cast<T*>(&storage));
		}

		template<typename T> requires(__detail::is_any_of_helper<T, Types...>::value)
		[[nodiscard]] T& get_unsafe() {
			return *std::launder(reinterpret_cast<T*>(&storage));
		}

		~variant() {
			if (id == SIZE_MAX) {
				return;
			}
			(destroy<Types>() || ...);
		}

	private:
		template<typename Visitor, typename T> requires requires(Visitor& visitor, T& arg) {
				visitor(arg);
			}
		inline bool visit_internal(Visitor& visitor) {
			constexpr size_t ID = __detail::index_of<T, Types...>::value;
			if (id == ID) {
				visitor(*std::launder(reinterpret_cast<T*>(&storage)));
				return true;
			}
			else {
				return false;
			}
		}
		template<typename Visitor, typename T>
		inline bool visit_internal(Visitor&) {
			return false;
		}

		template<typename T>
		inline bool destroy() {
			constexpr size_t ID = __detail::index_of<T, Types...>::value;
			if (id == ID) {
				std::launder(reinterpret_cast<T*>(&storage))->~T();
				return true;
			}
			else {
				return false;
			}
		}

		template<typename T>
		inline bool move_from(variant& other) {
			constexpr size_t ID = __detail::index_of<T, Types...>::value;
			if (other.id == ID) {
				auto* ptr = std::launder(reinterpret_cast<T*>(&other.storage));
				new (storage) T {std::move(*ptr)};
				id = ID;
				other.id = SIZE_MAX;
				return true;
			}
			else {
				return false;
			}
		}

		template<typename T>
		inline bool copy_from(const variant& other) {
			constexpr size_t ID = __detail::index_of<T, Types...>::value;
			if (other.id == ID) {
				auto* ptr = std::launder(reinterpret_cast<T*>(&other.storage));
				new (storage) T {*ptr};
				id = ID;
				return true;
			}
			else {
				return false;
			}
		}


		size_t id {};
		alignas(max({alignof(Types)...})) char storage[max({sizeof(Types)...})] {};
	};

	template<typename... Args>
	struct overloaded : Args... {
		using Args::operator()...;
	};
}
