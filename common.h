#pragma once
//Thordur Common

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <float.h>
#define _USE_MATH_DEFINES//MSVC
#include <math.h>

//gerum bara #define assert ef við viljum ekki assert
#if !defined(assert)
#include <assert.h>
#endif

#if defined(_MSC_VER)
	#include <intrin.h>
#endif

#ifndef _MSC_VER
#define tc_getc getc_unlocked
#else
#define tc_getc getc
#endif

#define U8_MAX UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX

#define S8_MAX INT8_MAX
#define S8_MIN INT8_MIN
#define S16_MAX INT16_MAX
#define S16_MIN INT16_MIN
#define S32_MAX INT32_MAX
#define S32_MIN INT32_MIN
#define S64_MAX INT64_MAX
#define S64_MIN INT64_MIN

#define F32_MIN FLT_MIN
#define F32_MAX FLT_MAX
#define F32_NEG_MAX -FLT_MAX
#define F64_MIN DBL_MIN
#define F64_MAX DBL_MAX
#define F64_NEG_MAX -DBL_MAX

#define absS32(x) abs(x)
#define absS64(x) llabs(x)
#define absF32(x) fabsf(x)
#define absF64(x) fabs(x)

#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value)*1024LL)
#define gigabytes(value) (megabytes(value)*1024LL)
#define terabytes(value) (gigabytes(value)*1024LL)

#define arrayLength(a) (sizeof(a)/sizeof(*a))

#define tc_alloc(size) calloc(size, 1)

#if !defined(_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++11-extensions"
#endif

namespace tc {
	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;

	typedef int8_t s8;
	typedef int16_t s16;
	typedef int32_t s32;
	typedef int64_t s64;

	typedef float f32;
	typedef double f64;


	inline u64 count_lz(u64 v) {
		#if defined(__GNUC__) || defined(__clang__)
			return __builtin_clzll(v);
		#elif defined(_MSC_VER)
			return __lzcnt64(v);
		#else
			#error clz intrinsic not found
		#endif
	}

	inline u64 count_tz(u64 v) {
		#if defined(__GNUC__) || defined(__clang__)
			return __builtin_ctzll(v);
		#elif defined(_MSC_VER)
			u64 result;
			_BitScanForward64((unsigned long *)&result, v);
			return result;
		#else
			#error ctz intrinsic not found
		#endif
	}

	inline u64 count_bits(u64 v) {
		#if defined(__GNUC__) || defined(__clang__)
			return __builtin_popcountll(v);
		#elif defined(_MSC_VER)
			return __popcnt64(v);
		#else
			#error popcount intrinsic not found
		#endif
	}

	//arena based memory allocator
	struct memRegion {
		char *data;
		size_t size;
		size_t used;

		memRegion(size_t bytes) {
			data = (char *)tc_alloc(bytes);
			size = bytes;
			used = 0;
		};

		void *alloc(size_t bytes) {
			assert(used + bytes < size);

			void *result = data + used;
			used += bytes;

			return result;
		};

		void clear() {
			used = 0;
		};
	};

	u64 sdbmHashBytes(const void *data, size_t size) {
		const char *ptr = (const char *)data;
		u64 hash = 0;

		for(s32 i = 0; i < size; i++) {
			hash = ptr[i] + (hash << 6) + (hash << 16) - hash;
		}

		return hash;
	}

	u64 sdbmHashStr(const char *str) {
		u64 hash = 0;
		u8 c;

		while((c = (u8)(*str++))) {
			hash = c + (hash << 6) + (hash << 16) - hash;
		}

		return hash;
	}

};

#if !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif
