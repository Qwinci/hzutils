#pragma once

#include "type_traits.hpp"

namespace std {
	namespace __detail {
		template<typename __T, typename __U>
		concept same_helper = is_same_v<__T, __U>;
	}

	template<typename __T, typename __U>
	concept same_as = __detail::same_helper<__T, __U> && __detail::same_helper<__U, __T>;
}
