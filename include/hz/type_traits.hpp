#pragma once

namespace hz {
	template<typename T>
	struct remove_cv {
		using type = T;
	};
	template<typename T>
	struct remove_cv<const T> {
		using type = T;
	};
	template<typename T>
	struct remove_cv<volatile T> {
		using type = T;
	};
	template<typename T>
	struct remove_cv<const volatile T> {
		using type = T;
	};

	template<typename T>
	struct remove_const {
		using type = T;
	};
	template<typename T>
	struct remove_const<const T> {
		using type = T;
	};

	template<typename T>
	struct remove_volatile {
		using type = T;
	};
	template<typename T>
	struct remove_volatile<volatile T> {
		using type = T;
	};

	template<typename T>
	using remove_cv_t = typename remove_cv<T>::type;

	template<typename T>
	using remove_const_t = typename remove_const<T>::type;

	template<typename T>
	using remove_volatile_t = typename remove_volatile<T>::type;

	template<typename T>
	struct remove_reference {
		using type = T;
	};
	template<typename T>
	struct remove_reference<T&> {
		using type = T;
	};
	template<typename T>
	struct remove_reference<T&&> {
		using type = T;
	};

	template<typename T>
	using remove_reference_t = typename remove_reference<T>::type;

	template<typename T>
	struct remove_cvref {
		using type = remove_cv_t<remove_reference_t<T>>;
	};

	template<typename T>
	using remove_cvref_t = remove_cvref<T>::type;

	template<typename T, T v>
	struct integral_constant {
		using value_type = T;
		using type = integral_constant<T, v>;

		static constexpr T value = v;

		constexpr operator value_type() const noexcept {
			return value;
		}

		constexpr value_type operator()() const noexcept {
			return value;
		}
	};

	template<bool v>
	using bool_constant = integral_constant<bool, v>;
	using true_type = integral_constant<bool, true>;
	using false_type = integral_constant<bool, false>;

	template<typename T>
	struct is_integral : bool_constant<requires(T value, T* ptr, void (*fn)(T)) {
		reinterpret_cast<T>(value);
		fn(0);
		ptr + value;
	}> {};

	template<typename T>
	inline constexpr bool is_integral_v = is_integral<T>::value;

	template<typename T>
	struct is_enum : bool_constant<__is_enum(T)> {};

	template<typename T>
	inline constexpr bool is_enum_v = is_enum<T>::value;

	template<typename T> requires(is_enum_v<T>)
	struct underlying_type {
		using type = __underlying_type(T);
	};

	template<typename T>
	using underlying_type_t = typename underlying_type<T>::type;

	template<typename T, typename U>
	struct is_same : false_type {};
	template<typename T>
	struct is_same<T, T> : true_type {};

	template<typename T, typename U>
	inline constexpr bool is_same_v = is_same<T, U>::value;

	template<typename T>
	struct is_pointer : false_type {};
	template<typename T>
	struct is_pointer<T*> : true_type {};
	template<typename T>
	struct is_pointer<const T*> : true_type {};
	template<typename T>
	struct is_pointer<volatile T*> : true_type {};
	template<typename T>
	struct is_pointer<const volatile T*> : true_type {};

	template<typename T>
	inline constexpr bool is_pointer_v = is_pointer<T>::value;
}
