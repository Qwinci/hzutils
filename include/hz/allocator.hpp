#pragma once
#include <stddef.h>
#include "concepts.hpp"

namespace hz {
	template<typename T>
	concept UnsizedAllocator =
		(same_as<decltype(&remove_reference_t<T>::allocate), void* (remove_reference_t<T>::*)(size_t)> ||
		same_as<decltype(&remove_reference_t<T>::allocate), void* (*)(size_t)>) &&
		(same_as<decltype(&remove_reference_t<T>::deallocate), void (remove_reference_t<T>::*)(void*)> ||
		same_as<decltype(&remove_reference_t<T>::deallocate), void (*)(void*)>);

	template<typename T>
	concept SizedAllocator =
	(same_as<decltype(&remove_reference_t<T>::allocate), void* (remove_reference_t<T>::*)(size_t)> ||
	same_as<decltype(&remove_reference_t<T>::allocate), void* (*)(size_t)>) &&
	(same_as<decltype(&remove_reference_t<T>::deallocate), void (remove_reference_t<T>::*)(void*, size_t)> ||
	same_as<decltype(&remove_reference_t<T>::deallocate), void (*)(void*, size_t)>);

	template<typename T>
	concept Allocator = UnsizedAllocator<T> || SizedAllocator<T>;
}
