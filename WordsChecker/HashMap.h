#pragma once

#include <stdbool.h>

typedef void* ValueType;
typedef struct { void* placeholder[0]; } HashMap;

HashMap *hashMap_create(void);

void hashMap_free(HashMap *const ptTree, const _Bool fFreeKeys);

_Bool hashMap_insert(HashMap *const pHashMap, char *const sKey, const ValueType value, const _Bool fCopyKey);

_Bool hashMap_get(const HashMap *const pHashMap, const char *const sKey, ValueType *const value);
