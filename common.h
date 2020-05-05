#if !defined(THORDUR_COMMON_H)
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <string.h>

#if !defined(assert)
#include <assert.h>
#endif

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

typedef const char cstring;

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

#define arrayLength(a) (sizeof(a)/sizeof(a)[0])

#define callocType(type, count) (type *)calloc(count, sizeof(type))
#define callocBytes(size) calloc(size, 1)
#define freeMemory(data) free(data)

void *reallocateMemory(void *memory, u64 current_size, u64 new_size) {
	void *result = 0;

	result = realloc(memory, new_size);
	if(new_size > current_size) {
		memset((u8 *)result + current_size, 0, new_size - current_size);
	}

	return result;
}

#define reallocType(type, memory, old_size, new_size) (type *)reallocateMemory(memory, old_size * sizeof(type), new_size * sizeof(type))

s32 isStreamEnd(FILE *in, s32 stream_end) {
	s32 c = getc_unlocked(in);
	if(c == stream_end) return 1;
	ungetc(c, in);
	return 0;
}

s32 isEOF(FILE *in) {
	return isStreamEnd(in, EOF);
}

s32 isWhitespace(char c) {
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

s64 stringParseInt(cstring *str) {
	s64 n = 0;
	s32 is_neg = 0;

	while(str) {
		char c = *str++;

		if(c == '-') {
			is_neg = 1;
			continue;
		}else if(c >= '0' && c <= '9') {
			n *= 10;
			n += c - '0';
		}else {
			break;
		}
	}

	if(is_neg) n = -n;

	return n;
}

s64 fileParseInt(FILE *stream) {
	s64 n = 0;
	s32 is_neg = 0;
	s32 first_char = 1;

	while(1) {
		s32 c = getc_unlocked(stream);

		if(first_char && c == '-') {
			is_neg = 1;
			first_char = 0;
		}else if(c >= '0' && c <= '9') {
			n *= 10;
			n += c - '0';
		}else {
			ungetc(c, stream);
			break;
		}
	}

	if(is_neg) n = -n;

	return n;
}

s32 fileReadToken(FILE *stream, char *buffer, cstring *sep, u64 *length_out) {
	char *start = buffer;
	s32 end_not_reached = 1;

	while(1) {
		s32 c = getc_unlocked(stream);

		if(c == EOF) {
			end_not_reached = 0;
			break;
		}

		for(s32 i = 0; sep[i]; i++) {
			if(c == sep[i]) goto done;
		}

		*buffer++ = c;
	}

done:
	if(length_out) *length_out = (u64)(buffer - start);
	*buffer = 0;

	return end_not_reached;
}

u32 sdbmHashBytes32(u32 n) {
	u32 hash = 0;

	hash = (n & 0xFF) + (hash << 6) + (hash << 16) - hash;
	hash = (n & 0xFF00) + (hash << 6) + (hash << 16) - hash;
	hash = (n & 0xFF0000) + (hash << 6) + (hash << 16) - hash;
	hash = (n & 0xFF000000) + (hash << 6) + (hash << 16) - hash;

	return hash;
}

u64 sdbmHashBytes(const char *data, size_t size) {
	u64 hash = 0;

	for(s32 i = 0; i < size; i++) {
		hash = data[i] + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

u64 sdbmHashStr(cstring *str) {
	u64 hash = 0;
	u8 c;

	while((c = (u8)(*str++))) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

s32 randomRange(s32 min, s32 max) {
	return (rand() % ((max - min)+1)) + min;
}

//stretchy buffer implementation
typedef struct bufferHeader bufferHeader;
struct bufferHeader {
	size_t length;
	size_t capacity;
	s8 data[0];
};

#define bufHeader(b) ((bufferHeader *)((s8 *)b - offsetof(bufferHeader, data)))
#define bufLen(b) ((b) ? bufHeader(b)->length : 0)
#define bufCap(b) ((b) ? bufHeader(b)->capacity : 0)
#define bufFit(b, n) ((bufLen(b) + (n) <= bufCap(b)) ? 0 : ((b) = bufferGrow((b), (bufLen(b) + (n)), sizeof(*(b)))))
#define bufPush(b, x) (bufFit(b, 1), b[bufLen(b)] = (x), bufHeader(b)->length++)
#define bufFree(b) ((b) ? freeMemory(bufHeader(b)) : 0)

void *bufferGrow(void *buffer, size_t increment, size_t element_size) {
	size_t double_capacity = 2*bufCap(buffer);
	size_t min_needed_cap = bufLen(buffer) + increment;
	size_t new_capacity = (double_capacity >= min_needed_cap) ? double_capacity : min_needed_cap;
	size_t new_header_size = offsetof(bufferHeader, data) + new_capacity * element_size;

	bufferHeader *new_header;

	//new_header = reallocType(bufferHeader, (buffer ? bufHeader(buffer) : 0), bufCap(buffer) * element_size, new_header_size);
	new_header = (bufferHeader *)realloc((buffer ? bufHeader(buffer) : 0), new_header_size);
	new_header->capacity = new_capacity;

	if(!buffer) {
		new_header->length = 0;
	}

	return new_header->data;
}

#ifdef __cplusplus
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++11-extensions"

template <typename T, size_t N> struct lister {
	T data[N] = {};
	size_t count = 0;

	T &operator[](size_t i) {
		assert(i >= 0 && i < N);
		return data[i];
	}
};

#define listerCap(l) (arrayLength((l)->data))

template <typename T, size_t N> T *operator+(lister<T, N> &a, size_t i) {
	assert(i >= 0 && i < arrayLength(a.data));
	return a.data + i;
}

template <typename T, size_t N> T *operator+(size_t i, lister<T, N> &a) {
	assert(i >= 0 && i < arrayLength(a.data));
	return a.data + i;
}

template <typename T, size_t N> T *listerPush(lister<T, N> *a, T element) {
	assert(a->count < listerCap(a));
	T *result = a->data + a->count;
	a->data[a->count++] = element;
	return result;
}

template <typename T, size_t N> void listerPop(lister<T, N> *a) {
	assert(a->count > 0);
	a->count--;
}

template <typename T, size_t N> void listerRemove(lister<T, N> *a, size_t i) {
	assert(a->count > 0);
	assert(i >= 0 && i < listerCap(a));
	a->data[i] = a->data[a->count-1];
	a->count--;
}

//hash table
template<typename K, typename V, size_t size> struct dict {
	u64 key_hashes[size] = {};
	V values[size] = {};
	const bool stringMode = false;
};

template<typename V, size_t size> struct dict<cstring *, V, size> {
	u64 key_hashes[size] = {};
	V values[size] = {};
	const bool stringMode = true;
};

#define dictCap(d) (arrayLength((d)->key_hashes))
#define dictHash(k, mode) ((mode) ? sdbmHashStr((k)) : sdbmHashBytes((const char *)&(k), sizeof((key))))
template<typename K, typename V, size_t size> void dictAdd(dict<K, V, size> *d, K key, V value, bool redef = false) {
	u64 hash = dictHash(key, d->stringMode);
	size_t capacity = dictCap(d);
	size_t index = hash % capacity;

	if(hash < 1) hash++;

	size_t i;
	for(i = 0; i < capacity; i++) {
		u64 key_hash = d->key_hashes[index];
		bool redefable = (hash == key_hash && redef);

		assert(hash != key_hash || redef);

		if(!key_hash || redefable) {
			d->key_hashes[index] = hash;
			d->values[index] = value;
			break;
		}

		index++;
		if(index == capacity) index = 0;
	}

	assert(i != capacity);
}

template<typename V, size_t size> void dictAdd(dict<cstring *, V, size> *d, char *key, V value, bool redef = false) {
	dictAdd(d, (cstring *)key, value, redef);
}


template<typename K, typename V, size_t size> V *dictGet(dict<K, V, size> *d, K key) {
	V *result = NULL;

	u64 hash = dictHash(key, d->stringMode);
	size_t capacity = dictCap(d);
	size_t index = hash % capacity;

	if(hash < 1) hash++;

	for(size_t i = 0; i < capacity; i++) {
		u64 key_hash = d->key_hashes[index];

		if(hash == key_hash) {
			result = d->values + index;
			break;
		}

		index++;
		if(index == capacity) index = 0;
	}

	return result;
}

template<typename V, size_t size> V *dictGet(dict<cstring *, V, size> *d, char *key) {
	return dictGet(d, (cstring *)key);
}


template<typename K, typename V, size_t size> V dictRemove(dict<K, V, size> *d, K key) {
	V *result = dictGet(d, key);
	assert(result);

	size_t index = (size_t)(result - d->values);
	d->key_hashes[index] = 0;
	d->values[index] = (V){0};

	return *result;
}

template<typename V, size_t size> V dictRemove(dict<cstring *, V, size> *d, char *key) {
	return dictRemove(d, (cstring *)key);
}

#pragma GCC diagnostic pop
#endif
#define THORDUR_COMMON_H
#endif
