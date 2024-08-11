#pragma once
#include "common.h"

namespace tc {
	template<size_t N> struct bitArray {
		static constexpr u32 BIT_ARRAY_WORD = 32;
		u32 data[N/BIT_ARRAY_WORD + 1] = {0};
		const size_t capacity = N;

		bool operator[](size_t i) {
			assert(i >= 0 && i < capacity);
			size_t internal_index = i / BIT_ARRAY_WORD;
			return data[internal_index] & (1 << (i & (BIT_ARRAY_WORD-1)));
		}

		void set(size_t i, bool val) {
			assert(i >= 0 && i < capacity);
			size_t internal_index = i / BIT_ARRAY_WORD;
			u32 mask = 1 << (i & (BIT_ARRAY_WORD-1));
			data[internal_index] = (val ? data[internal_index] | mask : data[internal_index] & ~mask);
		}
	};
};
