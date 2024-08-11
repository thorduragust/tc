#pragma once
#include "common.h"

namespace tc {
	#define DICT_KEY_NONE 0
	#define DICT_LOAD_MAX 13/16
	#define DICT_LOAD_MIN 4/16
	#define DICT_MIN_CAP 16

	template <typename K, typename V> struct dict {
		u64 *key_hashes = NULL;
		V *values = NULL;
		size_t capacity = 0;
		size_t count = 0;

		//TODO: hash differently if sizeof(T) <= 4
		template<typename T> u64 hashKey(T key) {
			u64 result = 0;

			result = sdbmHashBytes(&key, sizeof(key));

			if(result == DICT_KEY_NONE) result++;

			return result;
		}

		u64 hashKey(const char *key) {
			u64 result = 0;

			result = sdbmHashStr(key);

			if(result == DICT_KEY_NONE) result++;

			return result;
		}

		void resize(size_t new_capacity) {
			u64 *new_key_hashes = (u64 *)tc_alloc(new_capacity * sizeof(u64));
			V *new_values = (V *)tc_alloc(new_capacity * sizeof(V));

			for(size_t i = 0; i < capacity; i++) {
				u64 stored_hash = key_hashes[i];
				if(stored_hash != DICT_KEY_NONE) {
					size_t new_index = stored_hash & (new_capacity-1);
					if(new_key_hashes[new_index] != DICT_KEY_NONE) {
						for(size_t j = 0; j < new_capacity; j++) {
							new_index++;
							if(new_index == new_capacity) new_index = 0;
							if(new_key_hashes[new_index] == DICT_KEY_NONE) break;
						}
					}

					new_key_hashes[new_index] = stored_hash;
					new_values[new_index] = values[i];
				}
			}

			free(key_hashes);
			free(values);
			key_hashes = new_key_hashes;
			values = new_values;
			capacity = new_capacity;
		}

		//TODO: virkar þetta rétt?
		void reserve(size_t expected_count) {
			assert(capacity == 0);
			size_t threshold = (size_t)(expected_count * (1.0f/((f32)DICT_LOAD_MAX)));
			//find the closest power of 2
			/*
			size_t cap = threshold - 1;
			cap |= cap >> 1;
			cap |= cap >> 2;
			cap |= cap >> 4;
			cap |= cap >> 8;
			cap |= cap >> 16;
			cap |= cap >> 32;
			cap++;
			*/
			size_t cap = 1 << (64 - count_lz(threshold));
			resize(cap);
		}

		void add(K key, V value) {
			//Also works when the table is empty
			if(count >= capacity*DICT_LOAD_MAX) {
				size_t double_capacity = capacity*2;
				size_t new_capacity = double_capacity ? double_capacity : DICT_MIN_CAP;
				resize(new_capacity);
			}

			u64 hash = hashKey(key);
			size_t probe_index = hash & (capacity-1);

			for(size_t i = 0; i < capacity; i++) {
				u64 stored_hash = key_hashes[probe_index];
				assert(hash != stored_hash);

				if(stored_hash == DICT_KEY_NONE) {
					key_hashes[probe_index] = hash;
					values[probe_index] = value;
					count++;
					break;
				}

				probe_index++;
				if(probe_index == capacity) probe_index = 0;
			}
		}

		V *get(K key) {
			V *result = NULL;

			u64 hash = hashKey(key);
			size_t probe_index = hash & (capacity-1);

			for(size_t i = 0; i < capacity; i++) {
				u64 stored_hash = key_hashes[probe_index];

				if(stored_hash == DICT_KEY_NONE) break;

				if(hash == stored_hash) {
					result = values + probe_index;
					break;
				}

				probe_index++;
				if(probe_index == capacity) probe_index = 0;
			}

			return result;
		}

		bool remove(K key) {
			u64 hash = hashKey(key);
			size_t delete_index = hash & (capacity-1); //The slot to which the key should ideally hash to or the start of a chain
			size_t probe_index = delete_index;
			bool deleted = false;

			for(size_t i = 0; i < capacity; i++) {
				u64 stored_hash = key_hashes[probe_index];
				if(stored_hash == DICT_KEY_NONE) break;

				size_t true_index = stored_hash & (capacity-1);

				if(!deleted) {
					if(stored_hash == hash) {
						//Since we are deleting, the index which the key was found on becomes the slot to which
						//we need to move other elements in the chain to, if there are any.
						key_hashes[probe_index] = DICT_KEY_NONE;
						delete_index = probe_index;
						count--;
						deleted = true;

						if(capacity > DICT_MIN_CAP && count <= capacity*DICT_LOAD_MIN) {
							size_t new_capacity = capacity/2;
							resize(new_capacity);
							break;
						}
					}
				}else {
					if((probe_index > delete_index && (true_index <= delete_index || true_index > probe_index)) 
					|| (probe_index < delete_index && (true_index <= delete_index && true_index > probe_index))) {
						key_hashes[delete_index] = key_hashes[probe_index];
						values[delete_index] = values[probe_index];
						key_hashes[probe_index] = DICT_KEY_NONE;
						delete_index = probe_index;

					}
				}

				probe_index++;
				if(probe_index == capacity) probe_index = 0;
			}

			return deleted;
		}
	};
};
