#pragma once
#include "common.h"
#include <string.h>

namespace tc {

	//Basic wrapper around char []
	template<size_t N> struct string {
		size_t capacity = N;
		size_t length = 0;
		char data[N] = {0};

		void copy(const char *s) {
			size_t length_in = strlen(s);
			assert(length_in < capacity);
			length = length_in;
			strcpy(data, s);
		}

		template<size_t M> void copy(const string<M> &s) {
			if(s.data == data) return;
			assert(s.length < capacity);
			length = s.length;
			strcpy(data, s.data);
		}

		string() {}
		string(const char *s) { copy(s); };
		template<size_t M> string(string<M> &s) { copy(s); };

		string &operator=(const char *s) {
			copy(s);
			return *this;
		}

		template <size_t M> string &operator=(const string<M> &s) {
			copy(s);
			return *this;
		}

		bool operator==(const char *s) const {
			return (strcmp(data, s) == 0);
		}

		template <size_t M> bool operator==(const string<M> &s) const {
			return (strcmp(data, s.data) == 0);
		}

		s32 search(const char *s) {
			char *result = strstr(data, s);
			return (result ? (s32)(result - data) : -1);
		}

		template<size_t M> s32 search(const string<M> &s) {
			return search(s.data);
		}

		void append(const char *s) {
			size_t length_in = strlen(s);
			assert(length + length_in < capacity);
			strcpy(data + length, s);
			length += length_in;
		}

		template<size_t M> void append(const string<M> &s) {
			assert(length + s.length < capacity);
			strcpy(data + length, s.data);
			length += s.length;
		}

		void trim(const char *set) {
			char *s = data;
			for(char *ptr = data; *ptr; ptr++) {
				bool ignore = false;

				for(const char *set_s = set; *set_s; set_s++) {
					if(*ptr == *set_s) {
						ignore = true;
						break;
					}
				}

				if(!ignore) *s++ = *ptr;
			}

			*s = 0;
		}

		char &operator[](size_t i) {
			assert(i >= 0 && i < capacity);
			return data[i];
		}

		char *begin() {
			return data;
		}

		char *end() {
			return data + length;
		}
	};
};
