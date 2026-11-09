#pragma once

#include <stddef.h>

namespace std {
	enum class byte : unsigned char {};

	using nullptr_t = decltype(nullptr);
}
