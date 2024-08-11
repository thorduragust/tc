#pragma once
#include "common.h"

namespace tc {
	template<typename T, size_t N=0> struct array {
		T data[N] = {};
		size_t count = 0;
		const size_t capacity = N;

		T &operator[](size_t i) {
			assert(i >= 0 && i < capacity);
			return data[i];
		}

		T *begin() {
			return data;
		}

		T *end() {
			return data + count;
		}

		T *operator+(size_t i) {
			assert(i >= 0 && i < capacity);
			return data + i;
		}

		T *push(T element) {
			T *result = data + count;
			data[count++] = element;
			return result;
		}

		T pop() {
			assert(count > 0);
			return data[--count];
		}

		void removeOrdered(size_t i) {
			assert(count > 0);
			assert(i >= 0 && i < capacity);

			for(; i < count-1; i++) {
				data[i] = data[i+1];
			}

			count--;
		}

		void removeUnordered(size_t i) {
			assert(count > 0);
			assert(i >= 0 && i < capacity);
			data[i] = data[count-1];
			count--;
		}
	};

	//Partial specialization that is dynamically growing
	template<typename T> struct array<T, 0> {
		T *data = NULL;
		size_t count = 0;
		size_t capacity = 0;

		T &operator[](size_t i) {
			assert(i >= 0 && i < capacity);
			return data[i];
		}

		T *begin() {
			return data;
		}

		T *end() {
			return data + count;
		}

		T *operator+(size_t i) {
			assert(i >= 0 && i < capacity);
			return data + i;
		}

		//TODO: virkar þetta rétt?
		void reserve(size_t expected_count) {
			assert(capacity == 0);
			size_t new_capacity = 1 << (64 - count_lz(expected_count));
			data = (T *)realloc(data, new_capacity * sizeof(T));
			capacity = new_capacity;
		}

		T *push(T element) {
			if(count + 1 > capacity) {
				size_t double_capacity = capacity*2;
				size_t new_capacity = double_capacity ? double_capacity : 1;
				data = (T *)realloc(data, new_capacity * sizeof(T));
				capacity = new_capacity;
			}

			T *result = data + count;
			data[count++] = element;
			return result;
		}

		T pop() {
			assert(count > 0);
			return data[--count];
		}
	};
};
