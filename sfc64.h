#pragma once
#include "common.h"

namespace tc {
	struct sfc64 {
		u64 a;
		u64 b;
		u64 c;
		u64 d;

		static constexpr u32 p = 24;
		static constexpr u32 q = 11;
		static constexpr u32 r = 3;

		u64 next() {
			u64 result = a + b + d;
			d++;
			a = b ^ (b >> q);
			b = c + (c << r);
			c = ((c << p) | (c >> (sizeof(u64)*8 - p))) + result;
			return result;
		}

		void seed(u64 seed) {
			a = seed;
			b = seed;
			c = seed;
			d = 1;

			for(s32 i = 0; i < 12; i++) {
				next();
			}
		}

		sfc64(u64 seed) { seed(seed); }

		//Unbiased bitmask approach.
		//Find the closest power of 2 > range, reject if not within range.
		u64 bounded(u64 lo, u64 hi) {
			u64 range = hi - lo;
			u64 mask = ~((u64)0);
			range--;
			mask >>= count_lz(range|1);

			u64 result;

			do {
				result = next() & mask;
			} while(result > range);

			return result;
		}

		u64 bounded(u64 range) {
			return bounded(0, range);
		}

		f32 uniF32() {
			f32 float_n = (f32)(next() >> (64 - 24));
			return float_n * 0x1p-24f;
		}

		f64 uniF64() {
			f64 float_n = (f64)(next() >> (64 - 53));
			return float_n * 0x1p-53f;
		}
	};
}
