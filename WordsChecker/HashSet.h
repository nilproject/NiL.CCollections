#pragma once

#include <stdbool.h>

typedef struct { const _Bool fake; } HashSet;

HashSet *hashSet_create(void);

void hashSet_free(HashSet *const ptTree, const _Bool fFreeKeys);

_Bool hashSet_insert(HashSet *const pHashSet, char *const sKey, const _Bool fCopyKey);

_Bool hashSet_contains(const HashSet *const pHashSet, const char *const sKey);
