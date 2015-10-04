//
//  hash_table.c
//
//  Created by Jinglei Ren on Mar 5, 2014.
//  Copyright (c) 2014 Jinglei Ren <jinglei.ren@persper.com>
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "uthash.h"

struct entry {
  int key;
  char* value;
  UT_hash_handle hh;
};

#define K (1024)
#define M (1024 * K)

int main(int argc, const char* argv[]) {
  if (argc != 3) {
    printf("Usage: %s [SIZE in MBs] [VALUE SIZE]\n", argv[0]);
    return -1;
  }

  const uint64_t mbs = atol(argv[1]);
  const uint64_t max_size = mbs * M;
  const int value_size = atoi(argv[2]);
  const int num_keys = max_size / value_size;

  struct entry *store = NULL;
  char* const R = malloc(value_size);
  for (int i = 0; i < value_size; ++i) {
    R[i] = 'R';
  }

  for (int i = 0; i < num_keys; ++i) {
    int k = rand() % num_keys;
    struct entry *e;
    HASH_FIND_INT(store, &k, e);
    if (e) {
      assert(strncmp(e->value, R, value_size) == 0);
      HASH_DEL(store, e);
      free(e->value);
      free(e);
    } else {
      struct entry* e = malloc(sizeof(struct entry));
      e->value = malloc(value_size);
      e->key = k;
      strncpy(e->value, R, value_size);
      HASH_ADD_INT(store, key, e);
    }
  }

  printf("Final # entries: %d\n", HASH_COUNT(store));
}

