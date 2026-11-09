#pragma once

#include "config.hpp"

#if HZ_FREESTANDING
#include "type_traits.hpp"
#else
#include <type_traits>
#endif

#define __HZ_FLAGS_ENUM_IMPL(prefix, type) \
prefix constexpr type operator|(type lhs, type rhs) { \
	using u = std::underlying_type_t<type>; \
	return static_cast<type>(static_cast<u>(lhs) | static_cast<u>(rhs)); \
} \
\
prefix constexpr type& operator|=(type& lhs, type rhs) { \
	lhs = lhs | rhs; \
	return lhs; \
} \
\
prefix constexpr bool operator&(type lhs, type rhs) { \
	using u = std::underlying_type_t<type>; \
	return static_cast<u>(lhs) & static_cast<u>(rhs); \
} \
\
prefix constexpr type& operator&=(type& lhs, type rhs) { \
	using u = std::underlying_type_t<type>; \
	lhs = static_cast<type>(static_cast<u>(lhs) & static_cast<u>(rhs)); \
	return lhs; \
} \
\
prefix constexpr type operator~(type value) { \
	return static_cast<type>(~static_cast<std::underlying_type_t<type>>(value)); \
} \
static_assert(true)

#define HZ_FLAGS_ENUM(type) __HZ_FLAGS_ENUM_IMPL(, type)
#define HZ_FLAGS_ENUM_FRIEND(type) __HZ_FLAGS_ENUM_IMPL(friend, type)
