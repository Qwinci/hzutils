#pragma once

#include "config.hpp"

namespace std {
	template<typename __T>
	struct remove_reference {
		using type = __T;
	};
	template<typename __T>
	struct remove_reference<__T&> {
		using type = __T;
	};
	template<typename __T>
	struct remove_reference<__T&&> {
		using type = __T;
	};

	template<typename __T>
	using remove_reference_t = typename remove_reference<__T>::type;

	template<typename __T>
	struct remove_const {
		using type = __T;
	};
	template<typename __T>
	struct remove_const<const __T> {
		using type = __T;
	};

	template<typename __T>
	using remove_const_t = typename remove_const<__T>::type;

	template<typename __T>
	struct remove_cv {
		using type = __T;
	};
	template<typename __T>
	struct remove_cv<const __T> {
		using type = __T;
	};
	template<typename __T>
	struct remove_cv<volatile __T> {
		using type = __T;
	};
	template<typename __T>
	struct remove_cv<const volatile __T> {
		using type = __T;
	};

	template<typename __T>
	using remove_cv_t = typename remove_cv<__T>::type;

	template<typename __T>
	struct remove_cvref {
		using type = remove_cv_t<remove_reference_t<__T>>;
	};

	template<typename __T>
	using remove_cvref_t = remove_cvref<__T>::type;

	template<typename __T>
	struct underlying_type {
		using type = __underlying_type(__T);
	};

	template<typename __T>
	using underlying_type_t = typename underlying_type<__T>::type;

	template<typename __T>
	struct type_identity {
		using type = __T;
	};

	template<typename __T>
	using type_identity_t = typename type_identity<__T>::type;

#if hz_has_builtin(__make_unsigned)
	template<typename __T>
	struct make_unsigned {
		using type = __make_unsigned(__T);
	};
#else
	template<typename __T>
	struct make_unsigned {
		using type = __T;
	};
	template<>
	struct make_unsigned<char> {
		using type = unsigned char;
	};
	template<>
	struct make_unsigned<signed char> {
		using type = unsigned char;
	};
	template<>
	struct make_unsigned<short> {
		using type = unsigned short;
	};
	template<>
	struct make_unsigned<int> {
		using type = unsigned int;
	};
	template<>
	struct make_unsigned<long> {
		using type = unsigned long;
	};
	template<>
	struct make_unsigned<long long> {
		using type = unsigned long long;
	};
#endif

	template<typename __T>
	using make_unsigned_t = typename make_unsigned<__T>::type;

#if hz_has_builtin(__make_signed)
	template<typename __T>
	struct make_signed {
		using type = __make_signed(__T);
	};
#else
	template<typename __T>
	struct make_signed {
		using type = __T;
	};
	template<>
	struct make_signed<char> {
		using type = signed char;
	};
	template<>
	struct make_signed<unsigned char> {
		using type = signed char;
	};
	template<>
	struct make_signed<unsigned short> {
		using type = short;
	};
	template<>
	struct make_signed<unsigned int> {
		using type = int;
	};
	template<>
	struct make_signed<unsigned long> {
		using type = long;
	};
	template<>
	struct make_signed<unsigned long long> {
		using type = long long;
	};
#endif

	template<typename __T>
	using make_signed_t = typename make_signed<__T>::type;

	template<bool __V, typename __A, typename __B>
	struct conditional {
		using type = __A;
	};
	template<typename __A, typename __B>
	struct conditional<false, __A, __B> {
		using type = __B;
	};

	template<bool __V, typename __A, typename __B>
	using conditional_t = typename conditional<__V, __A, __B>::type;

	template<typename __T, __T __v>
	struct integral_constant {
		using value_type = __T;
		using type = integral_constant;

		static constexpr __T value = __v;

		constexpr operator value_type() const noexcept {
			return value;
		}

		constexpr value_type operator()() const noexcept {
			return value;
		}
	};

	template<bool __B>
	using bool_constant = integral_constant<bool, __B>;

	using true_type = integral_constant<bool, true>;
	using false_type = integral_constant<bool, false>;

	template<typename __T, typename __U>
	struct is_same : false_type {};
	template<typename __T>
	struct is_same<__T, __T> : true_type {};

	template<typename __T, typename __U>
	inline constexpr bool is_same_v = is_same<__T, __U>::value;

	template<typename __T>
	struct is_trivially_copyable : integral_constant<bool, __is_trivially_copyable(__T)> {};

	template<typename __T>
	inline constexpr bool is_trivially_copyable_v = is_trivially_copyable<__T>::value;

	template<typename __T, typename... __Args>
	struct is_constructible : integral_constant<bool, __is_constructible(__T, __Args...)> {};

	template<typename __T, typename... __Args>
	inline constexpr bool is_constructible_v = is_constructible<__T, __Args...>::value;

	template<typename __T>
	struct is_trivially_destructible : integral_constant<bool, __is_trivially_destructible(__T)> {};

	template<typename __T>
	inline constexpr bool is_trivially_destructible_v = is_trivially_destructible<__T>::value;

#if hz_has_builtin(__is_convertible)
	template<typename __From, typename __To>
	struct is_convertible : integral_constant<bool, __is_convertible(__From, __To)> {};
#else
	template<typename __From, typename __To>
	struct is_convertible : integral_constant<bool, __is_convertible_to(__From, __To) {};
#endif

	template<typename __From, typename __To>
	inline constexpr bool is_convertible_v = is_convertible<__From, __To>::value;

#if hz_has_builtin(__is_signed)
	template<typename __T>
	struct is_signed : integral_constant<bool, __is_signed(__T)> {};
#else
	namespace __detail {
		template<typename __T>
		struct is_signed : integral_constant<bool, __T(-1) < __T(0)> {};
	}

	template<typename __T>
	struct is_signed : __detail::is_signed<__T>::type {};
#endif

	template<typename __T>
	inline constexpr bool is_signed_v = is_signed<__T>::value;

	template<typename __T>
	struct is_const : false_type {};
	template<typename __T>
	struct is_const<const __T> : true_type {};

	template<typename __T>
	inline constexpr bool is_const_v = is_const<__T>::value;

	template<typename __T>
	struct is_pointer : false_type {};
	template<typename __T>
	struct is_pointer<__T*> : true_type {};
	template<typename __T>
	struct is_pointer<const __T*> : true_type {};
	template<typename __T>
	struct is_pointer<const volatile __T*> : true_type {};

	template<typename __T>
	inline constexpr bool is_pointer_v = is_pointer<__T>::value;

	template<typename __T>
	struct is_integral : bool_constant<
	    requires(__T __val, __T* __ptr, void (*__fn)(__T)) {
			reinterpret_cast<__T>(__val);
			__fn(0);
			__ptr + __val;
		}> {};

	template<typename __T>
	inline constexpr bool is_integral_v = is_integral<__T>::value;

	[[nodiscard]] inline constexpr bool is_constant_evaluated() noexcept {
		return __builtin_is_constant_evaluated();
	}
}
