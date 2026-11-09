#pragma once

namespace hz {
	template<typename T>
	struct default_delete {
		constexpr default_delete() = default;

		template<typename D>
		constexpr default_delete(default_delete<D>) {}

		void operator()(T* ptr) {
			delete ptr;
		}
	};
}
