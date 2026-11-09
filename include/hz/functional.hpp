#pragma once

#include "config.hpp"

#include <stddef.h>

#if HZ_FREESTANDING
#include "type_traits.hpp"
#include "utility.hpp"
#include "new.hpp"
#else
#include <type_traits>
#include <utility>
#include <new>
#endif

namespace hz {
	namespace __detail {
		template<typename R, bool NoEx, typename... Args>
		class function_ref_base {
		public:
			constexpr function_ref_base() = default;

			template<typename F>
			constexpr function_ref_base(F* f) noexcept {
				fn_ptr = [](Args&&... args, void* entity) noexcept(NoEx) -> R {
					auto f = reinterpret_cast<F*>(entity);
					if constexpr (std::is_same_v<R, void>) {
						(*f)(std::forward<Args>(args)...);
					}
					else {
						return (*f)(std::forward<Args>(args)...);
					}
				};
				entity = (void*) f;
			}

			template<typename F> requires (!std::is_same_v<std::remove_cvref_t<F>, function_ref_base>)
			constexpr function_ref_base(F&& f) noexcept {
				fn_ptr = [](Args&&... args, void* entity) noexcept(NoEx) -> R {
					auto f = static_cast<std::remove_reference_t<F>*>(entity);
					if constexpr (std::is_same_v<R, void>) {
						(*f)(std::forward<Args>(args)...);
					}
					else {
						return (*f)(std::forward<Args>(args)...);
					}
				};
				entity = (void*) &f;
			}

			constexpr R operator()(Args... args) const noexcept(NoEx) {
				return fn_ptr(std::forward<Args>(args)..., entity);
			}

		private:
			R (*fn_ptr)(Args&&... args, void* entity) noexcept(NoEx) {};
			void* entity {};
		};
	}

	template<typename...>
	class function_ref;

	template<typename R, typename... Args>
	class function_ref<R(Args...)> : public __detail::function_ref_base<R, false, Args...> {
	public:
		constexpr function_ref() = default;

		template<typename F>
		constexpr function_ref(F* f) noexcept : __detail::function_ref_base<R, false, Args...> {f} {}

		template<typename F> requires (!std::is_same_v<std::remove_cvref_t<F>, function_ref>)
		constexpr function_ref(F&& f) noexcept : __detail::function_ref_base<R, false, Args...> {std::forward<F>(f)} {}
	};

	template<typename R, typename... Args>
	class function_ref<R(Args...) noexcept> : public __detail::function_ref_base<R, true, Args...> {
	public:
		constexpr function_ref() = default;

		template<typename F>
		constexpr function_ref(F* f) noexcept : __detail::function_ref_base<R, true, Args...> {f} {}

		template<typename F> requires (!std::is_same_v<std::remove_cvref_t<F>, function_ref>)
		constexpr function_ref(F&& f) noexcept : __detail::function_ref_base<R, true, Args...> {std::forward<F>(f)} {}
	};

	template<typename R, typename... Args>
	class function_ref<R(Args...) const> : public __detail::function_ref_base<R, false, Args...> {
	public:
		constexpr function_ref() = default;

		template<typename F>
		constexpr function_ref(F* f) noexcept : __detail::function_ref_base<R, false, Args...> {f} {}

		template<typename F> requires (!std::is_same_v<std::remove_cvref_t<F>, function_ref>)
		constexpr function_ref(F&& f) noexcept : __detail::function_ref_base<R, false, Args...> {std::forward<F>(f)} {}
	};

	template<typename R, typename... Args>
	class function_ref<R(Args...) const noexcept> : public __detail::function_ref_base<R, true, Args...> {
	public:
		constexpr function_ref() = default;

		template<typename F>
		constexpr function_ref(F* f) noexcept : __detail::function_ref_base<R, true, Args...> {f} {}

		template<typename F> requires (!std::is_same_v<std::remove_cvref_t<F>, function_ref>)
		constexpr function_ref(F&& f) noexcept : __detail::function_ref_base<R, true, Args...> {std::forward<F>(f)} {}
	};

	template<size_t, typename...>
	class fixed_function;

	template<size_t S, typename R, typename... Args>
	class fixed_function<S, R(Args...)> {
	public:
		constexpr fixed_function() = default;

		template<typename F>
		fixed_function(F* f) noexcept {
			static_assert(sizeof(F*) <= S);
			new (storage) F* {f};
			invoke = [](Args&&... args, void* entity) -> R {
				if constexpr (std::is_same_v<R, void>) {
					(**static_cast<F**>(entity))(std::forward<Args>(args)...);
				}
				else {
					return (**static_cast<F**>(entity))(std::forward<Args>(args)...);
				}
			};
			copy = [](void* entity, void* dest_entity) {
				new (dest_entity) F* {*static_cast<F**>(entity)};
			};
			move = copy;
		}

		template<typename F> requires (!std::is_same_v<std::remove_cvref_t<F>, fixed_function>)
		fixed_function(F&& f) noexcept {
			using fn_type = std::remove_reference_t<F>;

			static_assert(sizeof(fn_type) <= S);
			new (storage) fn_type(std::forward<F>(f));
			invoke = [](Args&&... args, void* entity) -> R {
				fn_type* ptr = std::launder(static_cast<fn_type*>(entity));
				if constexpr (std::is_same_v<R, void>) {
					(*ptr)(std::forward<Args>(args)...);
				}
				else {
					return (*ptr)(std::forward<Args>(args)...);
				}
			};
			copy = [](void* entity, void* dest_entity) {
				fn_type* ptr = std::launder(static_cast<fn_type*>(entity));
				new (dest_entity) fn_type(*ptr);
			};

			if constexpr (std::is_trivially_copyable_v<fn_type>) {
				move = copy;
			}
			else {
				move = [](void* entity, void* dest_entity) {
					fn_type* ptr = std::launder(static_cast<fn_type*>(entity));
					new (dest_entity) fn_type(std::move(*ptr));
				};
			}

			if constexpr (!std::is_trivially_destructible_v<fn_type>) {
				destroy = [](void* entity) {
					fn_type* ptr = std::launder(static_cast<fn_type*>(entity));
					ptr->~fn_type();
				};
			}
		}

		fixed_function(const fixed_function& other)
			: invoke {other.invoke}, copy {other.copy}, move {other.move}, destroy {other.destroy} {

			if (other) {
				other.copy(const_cast<char*>(other.storage), storage);
			}
		}

		fixed_function(fixed_function&& other) noexcept
			: invoke {other.invoke}, copy {other.copy}, move {other.move}, destroy {other.destroy} {

			if (other) {
				other.move(other.storage, storage);
			}
		}

		fixed_function& operator=(const fixed_function& other) {
			if (destroy) {
				destroy(storage);
			}
			invoke = other.invoke;
			copy = other.copy;
			move = other.move;
			destroy = other.destroy;

			if (other) {
				other.copy(const_cast<char*>(other.storage), storage);
			}

			return *this;
		}

		fixed_function& operator=(fixed_function&& other) noexcept {
			if (destroy) {
				destroy(storage);
			}
			invoke = other.invoke;
			copy = other.copy;
			move = other.move;
			destroy = other.destroy;

			if (other) {
				other.move(other.storage, storage);
			}

			return *this;
		}

		R operator()(Args... args) const noexcept {
			return invoke(std::forward<Args>(args)..., const_cast<char*>(storage));
		}

		constexpr explicit operator bool() const noexcept {
			return invoke;
		}

		~fixed_function() {
			if (destroy) {
				destroy(storage);
			}
		}

	private:
		alignas(max_align_t) char storage[S];
		R (*invoke)(Args&&... args, void* entity) {};
		void (*copy)(void* entity, void* dest_entity) {};
		void (*move)(void* entity, void* dest_entity) {};
		void (*destroy)(void* entity) {};
	};

	template<typename... Args>
	using small_function = fixed_function<32, Args...>;
}
