#pragma once

#include <stdint.h>

#if defined(_MSC_VER) && !defined(__clang__)
#define HZ_USE_BUILTIN_SRC_LOC 0
#elif defined(HZ_SRC_LOC_BASE_FILE) || defined(HZ_SRC_LOC_FUNC)
#define HZ_USE_BUILTIN_SRC_LOC 0
#else
#define HZ_USE_BUILTIN_SRC_LOC 1
#endif

namespace std {
#if HZ_USE_BUILTIN_SRC_LOC
	class source_location {
		struct __impl {
			const char* _M_file_name;
			const char* _M_function_name;
			unsigned int _M_line;
			unsigned int _M_column;
		};

		using __builtin_type = decltype(__builtin_source_location());

	public:
		constexpr source_location() noexcept = default;

		static consteval source_location current(__builtin_type __ptr = __builtin_source_location()) noexcept {
			source_location __obj;
			__obj.__i = static_cast<const __impl*>(__ptr);
			return __obj;
		}

		[[nodiscard]] constexpr uint_least32_t line() const noexcept {
			if (__i) {
				return __i->_M_line;
			}
			else {
				return 0;
			}
		}

		[[nodiscard]] constexpr uint_least32_t column() const noexcept {
			if (__i) {
				return __i->_M_column;
			}
			else {
				return 0;
			}
		}

		[[nodiscard]] constexpr const char* file_name() const noexcept {
			if (__i) {
				return __i->_M_file_name;
			}
			else {
				return "";
			}
		}

		[[nodiscard]] constexpr const char* function_name() const noexcept {
			if (__i) {
				return __i->_M_function_name;
			}
			else {
				return "";
			}
		}

	private:
		const __impl* __i {};
	};
#else
	struct source_location {
		static consteval source_location current(
			uint_least32_t __line = __builtin_LINE(),
#if defined(__has_builtin) && __has_builtin(__builtin_COLUMN)
			uint_least32_t __column = __builtin_COLUMN(),
#else
			uint_least32_t __column = 0,
#endif
#if defined(HZ_SRC_LOC_BASE_FILE) && defined(__has_builtin) && __has_builtin(__builtin_FILE_NAME)
			const char* __file = __builtin_FILE_NAME(),
#else
			const char* __file = __builtin_FILE(),
#endif
#if defined(HZ_SRC_LOC_FUNC) || (!defined(__has_builtin) || !__has_builtin(__builtin_FUNCSIG))
			const char* __func = __builtin_FUNCTION()
#else
			const char* __func = __builtin_FUNCSIG()
#endif
		) {
			source_location __loc;
			__loc.__line = __line;
			__loc.__column = __column;
			__loc.__file = __file;
			__loc.__func = __func;
			return __loc;
		}

		[[nodiscard]] constexpr uint_least32_t line() const noexcept {
			return __line;
		}

		[[nodiscard]] constexpr uint_least32_t column() const noexcept {
			return __column;
		}

		[[nodiscard]] constexpr const char* file_name() const noexcept {
			return __file;
		}

		[[nodiscard]] constexpr const char* function_name() const noexcept {
			return __func;
		}

	private:
		uint_least32_t __line {};
		uint_least32_t __column {};
		const char* __file = "";
		const char* __func = "";
	};
#endif
}
