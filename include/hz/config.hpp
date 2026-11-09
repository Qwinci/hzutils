#pragma once

#ifndef HZ_FREESTANDING
#define HZ_FREESTANDING !__STDC_HOSTED__
#endif

#ifndef HZ_ASSERTIONS

#ifdef NDEBUG
#define HZ_ASSERTIONS 0
#else
#define HZ_ASSERTIONS 1
#endif

#endif

#if defined(_MSC_VER)
#define HZ_FORCEINLINE __forceinline
#else
#define HZ_FORCEINLINE [[gnu::always_inline]] inline
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define hz_likely(what) (what)
#define hz_unlikely(what) (what)
#define hz_has_builtin(what) 0
#else
#define hz_likely(what) __builtin_expect(!!(what), 1)
#define hz_unlikely(what) __builtin_expect(!!(what), 0)
#define hz_has_builtin(what) __has_builtin(what)
#endif
